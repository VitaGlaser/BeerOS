#pragma once

#include "asn/asn-core/algs.hpp"
#include "asn/asn-core/span.hpp"
#include "asn/asn-core/types.hpp"
#include "base_attribute.hpp"

namespace AsnPlus::Bluetooth
{
    enum class FrameType : u8
    {
        UNDEFINED,
        READ_ITEMS,
        READ_PART,
        WRITE_ITEMS,
        WRITE_PART,
        RESET_ITEMS,
    };

    struct FrameHeader
    {
        FrameType type     = FrameType::UNDEFINED;
        u8        value1   = 0;
        u8        value2   = 1;
        u8        reserved = 0;
    };

    template< typename VALUE_TYPE, size_t BUFFER_SIZE = 244 >
    class ArrayAttribute : public BaseAttribute
    {
    public:
        using CALLBACK_TYPE                          = std::function< void( u32 ) >;
        using Status                                 = u8;

        static constexpr const char TAG[]            = "ArrayAttribute";
        static constexpr u8         HEADER_SIZE      = sizeof( FrameHeader );
        static constexpr u16        ITEM_SIZE        = sizeof( VALUE_TYPE );
        static constexpr u8         MAX_MESSAGE_SIZE = BUFFER_SIZE;
        static constexpr u8         PART_SIZE        = MAX_MESSAGE_SIZE - HEADER_SIZE;
        static constexpr u8         PART_COUNT       = ( ITEM_SIZE + PART_SIZE - 1 ) / PART_SIZE;
        static constexpr u8         LAST_PART_SIZE   = ITEM_SIZE - PART_SIZE * ( PART_COUNT - 1 );
        static constexpr u8         ITEMS_PER_BUFFER = ( MAX_MESSAGE_SIZE - HEADER_SIZE ) / ITEM_SIZE;
        static constexpr bool       IS_MULTIPART     = PART_COUNT > 1;

        const u32 ITEM_COUNT;

    protected:
        VALUE_TYPE &  value;
        CALLBACK_TYPE write_callback;
        u8            _item_index    = 0;
        u8            _part_or_count = IS_MULTIPART ? 0 : 1;

    public:
        ArrayAttribute(
            VALUE_TYPE &  value,
            u8            value_count,
            ServiceBase & service,
            Properties    properties,
            ble_uuid128_t uuid,
            CALLBACK_TYPE write_callback = NULL
        ) :
            BaseAttribute { service, properties, uuid, NULL },
            ITEM_COUNT { value_count },
            value { value },
            write_callback { write_callback }
        {
        }

        ArrayAttribute(
            VALUE_TYPE &  value,
            ServiceBase & service,
            Properties    properties,
            ble_uuid128_t uuid,
            CALLBACK_TYPE write_callback = NULL
        ) :
            ArrayAttribute { value, 1, service, properties, uuid, write_callback }
        {
        }

        template< u8 COUNT >
        ArrayAttribute(
            VALUE_TYPE ( &value )[ COUNT ],
            ServiceBase & service,
            Properties    properties,
            ble_uuid128_t uuid,
            CALLBACK_TYPE write_callback = NULL
        ) :
            ArrayAttribute { *value, COUNT, service, properties, uuid, write_callback }
        {
        }

        ArrayAttribute(
            Span< VALUE_TYPE > value,
            ServiceBase &      service,
            Properties         properties,
            ble_uuid128_t      uuid,
            CALLBACK_TYPE      write_callback = NULL
        ) :
            ArrayAttribute { *value.begin(), (u8) value.size(), service, properties, uuid, write_callback }
        {
        }

    protected:
        Status on_read( ble_gatt_access_ctxt * context ) override
        {
            if ( ! BaseAttribute::is_authorized() ) return 0;

            fix_index_overflow( _item_index, _part_or_count );

            FrameHeader header = {
                IS_MULTIPART ? FrameType::READ_PART : FrameType::READ_ITEMS,
                _item_index,
                _part_or_count,
                0,
            };

            IBytes data = get_item_bytes( _item_index, _part_or_count, MAX_MESSAGE_SIZE );

            if ( IS_MULTIPART ) _part_or_count += 1;
            else _item_index += data.size() / ITEM_SIZE;

            Status result1 = copy_from( context, &header, HEADER_SIZE );
            Status result2 = copy_from( context, data.begin(), data.size() );

            return result1 ? result1 : result2;
        }

        Status on_write( ble_gatt_access_ctxt * context ) override
        {
            if ( ! BaseAttribute::is_authorized() ) return 0;

            const u16 buffer_size = context->om->om_len;
            u8        buffer[ buffer_size ];

            Status result = copy_to( context, buffer, buffer_size );

            if ( result != 0 ) return result;

            FrameHeader & header = *(FrameHeader *) buffer;

            switch ( header.type )
            {
                case FrameType::READ_ITEMS:
                    [[fallthrough]];
                case FrameType::READ_PART:
                    return on_index_change( header, buffer_size );
                case FrameType::WRITE_ITEMS:
                    [[fallthrough]];
                case FrameType::WRITE_PART:
                    return on_data_write( header, buffer_size, buffer );
                case FrameType::RESET_ITEMS:
                    return on_data_reset( header, buffer_size );
                default:
                    return 0;
            }
        }

        Status on_data_write( FrameHeader & header, u32 buffer_size, u8 * buffer )
        {
            u8 item          = header.value1;
            u8 part_or_count = header.value2;

            fix_index_overflow( item, part_or_count );

            IBytes data = get_item_bytes( item, part_or_count, buffer_size );
            memcpy( data.begin(), buffer + HEADER_SIZE, data.size() );

            u8 count = IS_MULTIPART ? ( part_or_count == PART_COUNT ) : part_or_count;
            call_write_callback( item, item + count );

            return 0;
        }

        Status on_data_reset( FrameHeader & header, u32 buffer_size )
        {
            if ( buffer_size != HEADER_SIZE ) return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;

            const u32 first = std::min< u32 >( ITEM_COUNT, header.value1 );
            const u32 last  = std::min< u32 >( ITEM_COUNT, first + header.value2 );

            for ( u32 i = first; i < last; i++ )
            {
                get_item( i ) = {};
            }

            call_write_callback( first, last );

            return 0;
        }

        Status on_index_change( FrameHeader & header, u32 buffer_size )
        {
            if ( buffer_size != HEADER_SIZE ) return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;

            _item_index    = header.value1;
            _part_or_count = header.value2;

            return 0;
        }

        void fix_index_overflow( u8 & item, u8 & part )
        {
            if ( IS_MULTIPART && part >= PART_COUNT )
            {
                part = 0;
                item++;
            }

            if ( item >= ITEM_COUNT )
            {
                item = 0;
            }
        }

        IBytes get_item_bytes( u8 & item, u8 & part_or_count, u8 buffer_size )
        {
            IBytes result {};

            if ( IS_MULTIPART )
            {
                u8   part    = part_or_count;
                bool is_last = part == ( PART_COUNT - 1 );

                result       = {
                    get_part_pointer( item, part ),
                    is_last ? LAST_PART_SIZE : PART_SIZE,
                };
            }
            else
            {
                u8 items_in_buffer = ( buffer_size - HEADER_SIZE ) / ITEM_SIZE;
                u8 items_remaining = ITEM_COUNT - item;
                u8 count           = Alg::multimin( part_or_count, items_in_buffer, items_remaining );

                result             = {
                    get_part_pointer( item, 0 ),
                    count * ITEM_SIZE,
                };
            }

            return result;
        }

        VALUE_TYPE & get_item( u8 item_index ) { return ( &value )[ item_index ]; }

        u8 * get_part_pointer( u8 item_index, u8 part_index )
        {
            return ( (u8 *) &value ) + ( item_index * ITEM_SIZE ) + ( part_index * PART_SIZE );
        }

        void call_write_callback( u8 first_item, u8 last_item )
        {
            if ( write_callback != NULL )
            {
                for ( u8 i = first_item; i < last_item; i++ )
                {
                    write_callback( i );
                }
            }
        }

    private:
        using Log = Logger< ModuleConfig::Ble::LOG_LEVEL, TAG >;
    };
}    // namespace AsnPlus::Bluetooth

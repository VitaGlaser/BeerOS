#pragma once

#include "asn/asn-core/algs.hpp"
#include "asn/asn-core/span.hpp"
#include "asn/asn-core/types.hpp"
#include "base_attribute.hpp"

namespace AsnPlus::Bluetooth
{
    enum class FrameType : uint8_t
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
        uint8_t   value1   = 0;    // item index
        uint8_t   value2   = 1;    // part index OR item count
        uint8_t   reserved = 0;
    };

    template< typename VALUE_TYPE, size_t BUFFER_SIZE = 244 >
    class ArrayAttribute : public BaseAttribute
    {
    public:
        using CALLBACK_TYPE                          = std::function< void( uint32_t ) >;
        using Status                                 = uint8_t;

        static constexpr const char TAG[]            = "ArrayAttribute";
        static constexpr uint8_t    HEADER_SIZE      = sizeof( FrameHeader );
        static constexpr uint16_t   ITEM_SIZE        = sizeof( VALUE_TYPE );
        static constexpr uint8_t    MAX_MESSAGE_SIZE = BUFFER_SIZE;
        static constexpr uint8_t    PART_SIZE        = MAX_MESSAGE_SIZE - HEADER_SIZE;
        static constexpr uint8_t    PART_COUNT       = ( ITEM_SIZE + PART_SIZE - 1 ) / PART_SIZE;
        static constexpr uint8_t    LAST_PART_SIZE   = ITEM_SIZE - PART_SIZE * ( PART_COUNT - 1 );
        static constexpr uint8_t    ITEMS_PER_BUFFER = ( MAX_MESSAGE_SIZE - HEADER_SIZE ) / ITEM_SIZE;
        static constexpr bool       IS_MULTIPART     = PART_COUNT > 1;

        const uint32_t ITEM_COUNT;

    protected:
        VALUE_TYPE &  value;
        CALLBACK_TYPE write_callback;
        uint8_t       _item_index    = 0;
        uint8_t       _part_or_count = IS_MULTIPART ? 0 : 1;

    public:
        /**
         * @brief Constructs the attribute from the first value and the value count.
         *
         * Example:
         * Config array[ 10 ];
         * ArrayAttribute< Config > attribute( array[2], 4, service, RW, ...uuid... );
         */
        ArrayAttribute(
            VALUE_TYPE &  value,
            uint8_t       value_count,
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

        /**
         * @brief Constructs the attribute from a single value.
         *
         * Example:
         * Config value;
         * ArrayAttribute< Config > attribute( value, service, RW, ...uuid... );
         */
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

        /**
         * @brief Constructs the attribute from a c-style array.
         *
         * Example:
         * Config array[ 10 ];
         * ArrayAttribute< Config > attribute( array, service, RW, ...uuid... );
         */
        template< uint8_t COUNT >
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

        /**
         * @brief Constructs the attribute from any container.
         *
         * Example:
         * Vector< Config, 10 > vector;
         * ArrayAttribute< Config > attribute( { vector }, service, RW, ...uuid... );
         */
        ArrayAttribute(
            Span< VALUE_TYPE > value,
            ServiceBase &      service,
            Properties         properties,
            ble_uuid128_t      uuid,
            CALLBACK_TYPE      write_callback = NULL
        ) :
            ArrayAttribute { *value.begin(), (uint8_t) value.size(), service, properties, uuid, write_callback }
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

            if ( IS_MULTIPART )
                _part_or_count += 1;
            else
                _item_index += data.size() / ITEM_SIZE;

            Status result1 = copy_from( context, &header, HEADER_SIZE );
            Status result2 = copy_from( context, data.begin(), data.size() );

            return result1 ? result1 : result2;
        }

        Status on_write( ble_gatt_access_ctxt * context ) override
        {
            if ( ! BaseAttribute::is_authorized() ) return 0;

            const uint16_t buffer_size = context->om->om_len;
            uint8_t        buffer[ buffer_size ];

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

        Status on_data_write( FrameHeader & header, uint32_t buffer_size, uint8_t * buffer )
        {
            uint8_t item          = header.value1;
            uint8_t part_or_count = header.value2;

            fix_index_overflow( item, part_or_count );

            IBytes data = get_item_bytes( item, part_or_count, buffer_size );
            memcpy( data.begin(), buffer + HEADER_SIZE, data.size() );

            // ! multipart              => (part_or_count) items changed
            // multipart && ! last part => 0 items changed
            // multipart && last part   => 1 item changed
            uint8_t count = IS_MULTIPART ? ( part_or_count == PART_COUNT ) : part_or_count;
            call_write_callback( item, item + count );

            return 0;
        }

        Status on_data_reset( FrameHeader & header, uint32_t buffer_size )
        {
            if ( buffer_size != HEADER_SIZE ) return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;

            const uint32_t first = std::min< uint32_t >( ITEM_COUNT, header.value1 );
            const uint32_t last  = std::min< uint32_t >( ITEM_COUNT, first + header.value2 );

            for ( uint32_t i = first; i < last; i++ )
            {
                get_item( i ) = {};
            }

            call_write_callback( first, last );

            return 0;
        }

        Status on_index_change( FrameHeader & header, uint32_t buffer_size )
        {
            if ( buffer_size != HEADER_SIZE ) return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;

            // call to fix_index_overflow isn't needed here
            // ... it's used only before the indexes are used, not when they're changed

            _item_index    = header.value1;
            _part_or_count = header.value2;

            return 0;
        }

        void fix_index_overflow( uint8_t & item, uint8_t & part )
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

        IBytes get_item_bytes( uint8_t & item, uint8_t & part_or_count, uint8_t buffer_size )
        {
            IBytes result {};

            if ( IS_MULTIPART )
            {
                uint8_t part    = part_or_count;
                bool    is_last = part == ( PART_COUNT - 1 );

                result          = {
                    get_part_pointer( item, part ),
                    is_last ? LAST_PART_SIZE : PART_SIZE,
                };
            }
            else
            {
                uint8_t items_in_buffer = ( buffer_size - HEADER_SIZE ) / ITEM_SIZE;
                uint8_t items_remaining = ITEM_COUNT - item;
                uint8_t count           = Alg::multimin( part_or_count, items_in_buffer, items_remaining );

                result                  = {
                    get_part_pointer( item, 0 ),
                    count * ITEM_SIZE,
                };
            }

            return result;
        }

        VALUE_TYPE & get_item( uint8_t item_index ) { return ( &value )[ item_index ]; }

        uint8_t * get_part_pointer( uint8_t item_index, uint8_t part_index )
        {
            return ( (uint8_t *) &value ) + ( item_index * ITEM_SIZE ) + ( part_index * PART_SIZE );
        }

        void call_write_callback( uint8_t first_item, uint8_t last_item )
        {
            if ( write_callback != NULL )
            {
                for ( uint8_t i = first_item; i < last_item; i++ )
                {
                    write_callback( i );
                }
            }
        }

    private:
        using Log = Logger< ModuleConfig::Ble::LOG_LEVEL, TAG >;
    };
}    // namespace AsnPlus::Bluetooth

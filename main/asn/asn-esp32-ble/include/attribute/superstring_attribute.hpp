#pragma once

#include "../service/service.hpp"
#include "../uuid.hpp"
#include "asn/asn-core/logger.hpp"
#include "string_attribute.hpp"
#include <functional>

namespace AsnPlus::Bluetooth
{
    template< u32 CAPACITY >
    class SuperStringAttribute : public BaseAttribute
    {
    protected:
        static constexpr const char TAG[]                 = "SuperStringAttribute";
        using Log                                         = Logger< ModuleConfig::Ble::LOG_LEVEL, TAG >;

        static constexpr const u8 PACKET_SIZE             = 128;
        static constexpr const u8 LAST_MESSAGE_FLAG_MASK  = 0b10000000;
        bool                      _write_message_complete = true;
        u8                        _write_page_index       = 0;

        bool _read_message_complete                       = true;
        u32  _read_index                                  = 0;

    public:
        char & value;

        SuperStringAttribute(
            char &        value,
            ServiceBase & service,
            Properties    properties,
            ble_uuid128_t uuid,
            CALLBACK_TYPE write_callback = NULL
        ) :
            BaseAttribute { service, properties, uuid, write_callback },
            value { value }
        {
        }

        bool write_message_complete() { return _write_message_complete && _write_page_index != 0; }

        u8 on_read( ble_gatt_access_ctxt * ctxt ) override
        {
            if ( ! BaseAttribute::is_authorized() ) return 0;

            u16 message_length = strlen( &value ) - _read_index;

            _read_message_complete = ( message_length <= PACKET_SIZE - 1 );

            if ( ! _read_message_complete )
            {
                message_length = PACKET_SIZE - 1;
            }

            u8 message_index = _read_index / ( PACKET_SIZE - 1 ) + _read_message_complete * LAST_MESSAGE_FLAG_MASK;

            u8 result = copy_from( ctxt, &message_index, sizeof( message_index ) );
            if ( result != 0 )
            {
                return result;
            }

            result = copy_from( ctxt, &value + _read_index, message_length );

            if ( _read_message_complete )
            {
                _read_index = 0;
            }
            else
            {
                _read_index += message_length;
            }

            return result;
        }

        u8 on_write( ble_gatt_access_ctxt * ctxt ) override
        {
            if ( ! BaseAttribute::is_authorized() ) return 0;

            char buffer[ PACKET_SIZE ] { 0 };
            u8   result = copy_to( ctxt, buffer, sizeof( buffer ) );
            if ( result != 0 )
            {
                return result;
            }

            if ( _write_message_complete )
            {
                memset( &value, 0, CAPACITY );
                if ( buffer[ 0 ] == 0 )
                {
                    _write_message_complete = false;
                    _write_page_index       = 0;
                }
                else if ( buffer[ 0 ] & LAST_MESSAGE_FLAG_MASK )
                {
                    _write_message_complete = true;
                }
            }
            else
            {
                if ( buffer[ 0 ] & LAST_MESSAGE_FLAG_MASK )
                {
                    _write_message_complete = true;
                }
                else if ( _write_page_index != ( buffer[ 0 ] - 1 ) )
                {
                    memset( &value, 0, CAPACITY );
                    _write_message_complete = true;
                    return 255;
                }
            }

            _write_page_index = buffer[ 0 ];
            strcat( &value, &buffer[ 1 ] );

            return 0;
        }
    };
}    // namespace AsnPlus::Bluetooth

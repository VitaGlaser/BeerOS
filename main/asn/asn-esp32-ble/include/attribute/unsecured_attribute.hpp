#pragma once

#include "base_attribute.hpp"

namespace AsnPlus::Bluetooth
{
    template< typename VALUE_TYPE >
    class UnsecuredSimpleAttribute : public BaseAttribute
    {
    public:
        VALUE_TYPE & value;

        UnsecuredSimpleAttribute(
            VALUE_TYPE &  value,
            ServiceBase & service,
            Properties    properties,
            ble_uuid128_t uuid,
            CALLBACK_TYPE write_callback = NULL
        ) :
            BaseAttribute { service, properties, uuid, write_callback },
            value { value }
        {
        }

        u8 on_read( ble_gatt_access_ctxt * context ) override
        {
            return BaseAttribute::copy_from( context, &value, sizeof( VALUE_TYPE ) );
        }

        u8 on_write( ble_gatt_access_ctxt * context ) override
        {
            return BaseAttribute::copy_to( context, &value, sizeof( VALUE_TYPE ) );
        }

        void notify() { BaseAttribute::notify( value ); }
    };
}    // namespace AsnPlus::Bluetooth

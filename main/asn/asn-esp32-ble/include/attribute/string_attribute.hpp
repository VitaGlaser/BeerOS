#pragma once

#include "base_attribute.hpp"

namespace AsnPlus::Bluetooth
{
    template< u32 CAPACITY >
    class StringAttribute : public BaseAttribute
    {
    public:
        char & value;

        StringAttribute(
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

        u8 on_read( ble_gatt_access_ctxt * context ) override
        {
            if ( ! BaseAttribute::is_authorized() ) return 0;

            return BaseAttribute::copy_from( context, &value, strlen( &value ) );
        }

        u8 on_write( ble_gatt_access_ctxt * context ) override
        {
            if ( ! BaseAttribute::is_authorized() ) return 0;

            memset( &value, 0, CAPACITY );
            return BaseAttribute::copy_to( context, &value, CAPACITY );
        }

        void notify() { BaseAttribute::notify( &value, strlen( &value ) ); }
    };
}    // namespace AsnPlus::Bluetooth

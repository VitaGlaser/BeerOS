#pragma once

#include "base_attribute.hpp"

namespace AsnPlus::Bluetooth
{
    template< typename VALUE_TYPE >
    class LambdaAttribute : public BaseAttribute
    {
    public:
        using MEMBER_SELECTOR = std::function< VALUE_TYPE *() >;

        MEMBER_SELECTOR get_member;

        LambdaAttribute(
            MEMBER_SELECTOR get_member,
            ServiceBase &   service,
            Properties      properties,
            ble_uuid128_t   uuid,
            CALLBACK_TYPE   write_callback = NULL
        ) :
            BaseAttribute { service, properties, uuid, write_callback },
            get_member { get_member }
        {
        }

        VALUE_TYPE & get_value() { return *get_member(); }

        u8 on_read( ble_gatt_access_ctxt * context ) override
        {
            if ( ! BaseAttribute::is_authorized() ) return 0;

            return copy_from( context, &get_value(), sizeof( VALUE_TYPE ) );
        }

        u8 on_write( ble_gatt_access_ctxt * context ) override
        {
            if ( ! BaseAttribute::is_authorized() ) return 0;

            return copy_to( context, &get_value(), sizeof( VALUE_TYPE ) );
        }
    };
}    // namespace AsnPlus::Bluetooth

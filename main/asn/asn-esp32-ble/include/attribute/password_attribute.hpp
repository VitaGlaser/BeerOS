#pragma once

#include "base_attribute.hpp"
#include <functional>

namespace AsnPlus::Bluetooth
{
    template< typename VALUE_TYPE >
    class PasswordAttribute : public BaseAttribute
    {
    public:
        using VALIDATOR_TYPE = std::function< bool( VALUE_TYPE & ) >;

    protected:
        VALIDATOR_TYPE validator;

    public:
        PasswordAttribute(
            VALIDATOR_TYPE validator,
            ServiceBase &  service,
            Properties     properties,
            ble_uuid128_t  uuid,
            CALLBACK_TYPE  write_callback = NULL
        ) :
            BaseAttribute { service, properties, uuid, write_callback },
            validator { validator }
        {
        }

        u8 on_write( ble_gatt_access_ctxt * context ) override
        {
            VALUE_TYPE value;
            u8         result = copy_to( context, &value, sizeof( value ) );

            BaseAttribute::authorize( validator( value ) );

            return result;
        }

        u8 on_read( ble_gatt_access_ctxt * context ) override
        {
            u8 state = BaseAttribute::is_authorized();
            return copy_from( context, &state, sizeof( state ) );
        }
    };
}    // namespace AsnPlus::Bluetooth

#pragma once

#include "../service/service.hpp"
#include "../version.hpp"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "properties.hpp"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "asn_module_config.hpp"

#include "asn/asn-core/logger.hpp"

namespace AsnPlus::Bluetooth
{
    class BaseAttribute
    {
    protected:
        u16 _conn_handle = 0;
        u16 _val_handle  = 0;

    public:
        using CALLBACK_TYPE = std::function< void() >;

        Properties    properties;
        ble_uuid128_t uuid;
        CALLBACK_TYPE write_callback;

        static bool is_authorized();
        static void authorize( bool is_authorized );

        BaseAttribute(
            ServiceBase & service,
            Properties    properties,
            ble_uuid128_t uuid,
            CALLBACK_TYPE write_callback = NULL
        );

        ble_gatt_chr_def characteristic();

        template< typename VALUE_TYPE >
        void notify( VALUE_TYPE value )
        {
            notify( &value, sizeof( value ) );
        }

        void notify( const void * buffer, const u32 length );

        u8 copy_from( ble_gatt_access_ctxt * ctxt, void * source, u32 length );
        u8 copy_to( ble_gatt_access_ctxt * ctxt, void * target, u32 length );

        virtual u8 on_read( ble_gatt_access_ctxt * ctxt );
        virtual u8 on_write( ble_gatt_access_ctxt * ctxt );

    protected:
        static int _callback( u16 conn_handle, u16 attr_handle, ble_gatt_access_ctxt * ctxt, void * arg );
        static bool & _authorized();
    };
}    // namespace AsnPlus::Bluetooth

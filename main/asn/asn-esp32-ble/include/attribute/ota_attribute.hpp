#pragma once

#include "../service/service.hpp"
#include "asn/asn-core/types.hpp"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "ota_update.hpp"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

namespace AsnPlus::Bluetooth
{
    class OtaAttribute
    {
    public:
        OtaUpdate &   _ota_update;
        ble_uuid128_t uuid;

        OtaAttribute( OtaUpdate & ota_update, ServiceBase & service, ble_uuid128_t uuid, u16 * val_handle = nullptr );

        ble_gatt_chr_def characteristic( u16 * val_handle );

        virtual int callback( u16 conn_handle, u16 attr_handle, ble_gatt_access_ctxt * ctxt );

    protected:
        static int _write_characteristic( os_mbuf * om, u16 min_len, u16 max_len, void * dst, u16 * len );
        static int _read_characteristic( os_mbuf * om, const void * data, u16 len );
        static int _callback( u16 conn_handle, u16 attr_handle, ble_gatt_access_ctxt * ctxt, void * arg );
    };

    class OtaControlAttribute : public OtaAttribute
    {
    public:
        OtaControlAttribute( OtaUpdate & ota_update, ServiceBase & service, ble_uuid128_t uuid );

        int callback( u16 conn_handle, u16 attr_handle, ble_gatt_access_ctxt * ctxt ) override;
    };

    class OtaDataAttribute : public OtaAttribute
    {
    public:
        OtaDataAttribute( OtaUpdate & ota_update, ServiceBase & service, ble_uuid128_t uuid );

        int callback( u16 conn_handle, u16 attr_handle, ble_gatt_access_ctxt * ctxt ) override;
    };
}    // namespace AsnPlus::Bluetooth

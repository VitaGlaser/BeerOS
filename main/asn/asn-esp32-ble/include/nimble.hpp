#pragma once

#include "asn_module_config.hpp"

#include "host/ble_gap.h"
#include "host/ble_hs.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "adv_data.hpp"
#include "attribute/base_attribute.hpp"
#include "service/service.hpp"
#include "structs.hpp"

#include "asn/asn-core/logger.hpp"

namespace AsnPlus::Bluetooth
{
    class Nimble
    {
    public:
        Nimble( AdvData & advData, Config & config, State & state );

        void initialize();
        static void host_task( void * pvParameters );
        bool start();
        void advertise();
        void disconnct();

    private:
        static constexpr const char TAG[] = "Nimble";
        using Log                         = Logger< ModuleConfig::Ble::LOG_LEVEL, TAG >;

        AdvData & _advData;
        Config &  _config;
        State &   _state;

        static int handle_event_cb( ble_gap_event * event, void * arg );
        int        handle_event( ble_gap_event & event );
    };
}    // namespace AsnPlus::Bluetooth

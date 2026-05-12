#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/array.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "asn/asn-hal/include/wifi/sta.hpp"

#include "esp_coexist.h"
#include "esp_wifi.h"

namespace AsnPlus::Wifi
{
    class Sta : public ISta
    {
    public:
        using Runtime         = ISta::Runtime;
        using ScannedNetworks = ISta::ScannedNetworks;

        Sta( Runtime & runtime, ScannedNetworks & scannedNetworks ) : ISta( runtime, scannedNetworks ) {}

        bool initialize() override;
        void poll() override;
        void startScan() override;
        void stopScan() override;
        void connect( WifiConfig & config ) override;
        void disconnect() override;

    private:
        static constexpr const char TAG[]   = "Sta";
        using Log                           = Logger< ModuleConfig::Wifi::LOG_LEVEL, TAG >;

        static const int32_t RSSI_THRESHOLD = -85;

        static AuthMode _fromAuthMode( wifi_auth_mode_t mode );
        static void     _eventHandler( void * arg, esp_event_base_t eventBase, int32_t eventId, void * eventData );

        void _fetchScannedNetworks();
        void _updateRssi();
    };
}    // namespace AsnPlus::Wifi

#pragma once

#include "asn/asn-core/string.hpp"

#include "../attribute/array_attribute.hpp"
#include "../attribute/command_attribute.hpp"
#include "../attribute/simple_attribute.hpp"
#include "../uuid.hpp"
#include "service.hpp"

#include "asn/asn-esp32-wifi/old/wifi_config.hpp"

namespace AsnPlus::Bluetooth
{
    class WifiService
    {
    public:
        Service< 6 > svc;

        WifiService( Wifi::LegacyWifiConfig & wifi_config );

    protected:
        static constexpr ble_uuid128_t SERVICE_UUID          = "52a08919-9e40-43e6-acd9-266137021433"_uuid;
        static constexpr ble_uuid128_t STATUS_UUID           = "0f121af2-f3a0-4c2d-a09a-fff2d9b4d611"_uuid;
        static constexpr ble_uuid128_t COMMAND_UUID          = "48d9033a-dd5e-41e6-985c-9aecc16e40c6"_uuid;
        static constexpr ble_uuid128_t SAVED_NETWORKS_UUID   = "4f995cd6-e7e6-43f5-aff9-7ab3bf467927"_uuid;
        static constexpr ble_uuid128_t SCANNED_NETWORKS_UUID = "c71fa1dd-13c3-4e71-9ab5-d10cbe30b55a"_uuid;

        Wifi::LegacyWifiConfig & _wifi_config;

        ArrayAttribute< Wifi::WifiStatus >         attr_status;
        CommandAttribute                           attr_command;
        ArrayAttribute< Wifi::SavedNetworkInfo >   attr_saved_wifis;
        ArrayAttribute< Wifi::ScannedNetworkInfo > attr_scanned_networks;
    };
}    // namespace AsnPlus::Bluetooth

#ifndef _ASNPLUS_WIFI_CONFIG_HPP
#define _ASNPLUS_WIFI_CONFIG_HPP

#include "asn/asn-esp32-hal/old/nvs.hpp"
#include "esp_wifi.h"

#include "asn/asn-core/array.hpp"
#include "asn/asn-core/string.hpp"
#include "asn/asn-core/string_util.hpp"
#include "asn/asn-core/types.hpp"

// TODO: Use etl::string instead of char[]
// TODO: Use etl::map instead of asn::array (map is internally a binary tree that might require some
// serialization/deserialization)

namespace AsnPlus::Wifi
{
    enum StaStatus : u8
    {
        SCANNING,
        SCAN_DONE,
        CONNECTING,
        CONNECTED,
        DISCONNECTING,
        DISCONNECTED,
        ERROR
    };

    enum Commands : u16
    {
        NO_COMMAND,
        CONFIG_START,
        CONFIG_END,
        SCAN,
        CONNECT,
        DISCONNECT
    };

    static const u8 SSID_LENGTH     = 32;
    static const u8 RSSI_LENGTH     = 3;
    static const u8 PASSWORD_LENGTH = 64;

    struct WifiStatus
    {
        i16                       rssi        = 0;
        StaStatus                 sta_status  = StaStatus::DISCONNECTED;
        bool                      config_mode = false;
        StringData< SSID_LENGTH > connected_network_ssid {};
    };

    struct ScannedNetworkInfo
    {
        i16                       rssi         = 0;
        bool                      has_password = 0;
        StringData< SSID_LENGTH > ssid {};
    };

    struct SavedNetworkInfo
    {
        StringData< SSID_LENGTH >     ssid {};
        StringData< PASSWORD_LENGTH > password {};
    };

    struct LegacyWifiConfig
    {
        Commands command                 = Commands::NO_COMMAND;
        // Default preffered MTU (256)
        static const u16 CMD_DATA_LENGTH = 128;
        u8               command_data[ CMD_DATA_LENGTH ] {};

        WifiStatus status;

        static const u8 MAX_SAVED = 8;
        // SSID1; ... SSIDn; + \0
        Array< SavedNetworkInfo, MAX_SAVED > saved_networks;

        static const u8 MAX_SCANNED = 32;
        // SSID1,RSSI1,has_password1; ... SSIDn,RSSI1,has_passwordn; + \0
        Array< ScannedNetworkInfo, MAX_SCANNED > scanned_networks {};

        esp_err_t load_saved_networks()
        {
            esp_err_t ret = Esp32::Nvs::load_config( saved_networks, "saved_networks" );
            return ret;
        }

        esp_err_t store_saved_networks()
        {
            esp_err_t ret = Esp32::Nvs::store_config( saved_networks, "saved_networks" );
            return ret;
        }
    };

}    // namespace AsnPlus::Wifi

#endif

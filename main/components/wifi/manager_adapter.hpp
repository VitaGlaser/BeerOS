#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/logger.hpp"

#include "asn/asn-esp32-wifi/old/wifi_config.hpp"
#include "asn/asn-esp32-wifi/include/wifi/sta.hpp"
#include "asn/asn-hal/include/wifi/manager.hpp"

#include "esp_coexist.h"
#include "esp_netif.h"

#include <cstring>

namespace AsnPlus::Wifi
{
    // Adapter that keeps the legacy LegacyWifiConfig-based external API while
    // driving the new Manager + ISta internals. All state is synced on every poll().
    class WifiManager
    {
    public:
        explicit WifiManager( LegacyWifiConfig & legacyConfig ) :
            _legacyConfig( legacyConfig ),
            _sta( _staRuntime, _scannedNetworks ),
            _manager( _managerConfig, _request, _managerRuntime, _sta, _staRuntime, _scannedNetworks, _savedNetworks )
        {
        }

        void initialize()
        {
            esp_coex_preference_set( ESP_COEX_PREFER_WIFI );

            if ( _legacyConfig.load_saved_networks() != ESP_OK )
                _legacyConfig.store_saved_networks();

            _syncSavedNetworksFromLegacy();
            _manager.initialize();
        }

        void poll()
        {
            _syncSavedNetworksFromLegacy();
            _syncCommandFromLegacy();
            _manager.poll();
            _syncStateToLegacy();
            _syncScannedNetworksToLegacy();
        }

        bool is_connected() const { return _staRuntime.state == ISta::State::CONNECTED; }

        esp_netif_t * getNetif() { return esp_netif_get_handle_from_ifkey( "WIFI_STA_DEF" ); }

    private:
        static constexpr const char TAG[] = "WifiManager";
        using Log                         = Logger< ModuleConfig::Wifi::LOG_LEVEL, TAG >;

        LegacyWifiConfig & _legacyConfig;

        Manager::Config        _managerConfig {};
        Manager::Request       _request {};
        Manager::Runtime       _managerRuntime {};
        ISta::Runtime          _staRuntime {};
        ISta::ScannedNetworks  _scannedNetworks {};
        Manager::SavedNetworks _savedNetworks {};

        Sta     _sta;
        Manager _manager;

        void _syncCommandFromLegacy()
        {
            switch ( _legacyConfig.command )
            {
                case Commands::NO_COMMAND:
                    return;
                case Commands::CONFIG_START:
                    _request.command = Manager::Command::CONFIG_START;
                    break;
                case Commands::CONFIG_END:
                    _request.command = Manager::Command::CONFIG_END;
                    break;
                case Commands::SCAN:
                    _request.command = Manager::Command::SCAN;
                    break;
                case Commands::CONNECT:
                    {
                        _request.command      = Manager::Command::CONNECT;
                        const char * ssid     = reinterpret_cast< const char * >( _legacyConfig.command_data );
                        const char * password = ssid + strlen( ssid ) + 1;
                        WifiConfig   network {};
                        network.ssid.assign( ssid );
                        network.password.assign( password );
                        network.serialize( _request.commandData );
                        break;
                    }
                case Commands::DISCONNECT:
                    _request.command = Manager::Command::DISCONNECT;
                    break;
                default:
                    return;
            }
            _legacyConfig.command = Commands::NO_COMMAND;
        }

        void _syncSavedNetworksFromLegacy()
        {
            for ( uint8_t i = 0; i < ModuleConfig::Wifi::MAX_SAVED_NETWORKS; ++i )
            {
                const SavedNetworkInfo & src = _legacyConfig.saved_networks[ i ];
                WifiConfig &             dst = _savedNetworks[ i ];
                dst.ssid.assign( src.ssid.data );
                dst.password.assign( src.password.data );
            }
        }

        void _syncStateToLegacy()
        {
            switch ( _staRuntime.state )
            {
                case ISta::State::SCANNING:
                    _legacyConfig.status.sta_status = StaStatus::SCANNING;
                    break;
                case ISta::State::SCAN_DONE:
                    _legacyConfig.status.sta_status = StaStatus::SCAN_DONE;
                    break;
                case ISta::State::CONNECTING:
                    _legacyConfig.status.sta_status = StaStatus::CONNECTING;
                    break;
                case ISta::State::CONNECTED:
                    _legacyConfig.status.sta_status = StaStatus::CONNECTED;
                    _legacyConfig.status.rssi       = static_cast< int16_t >( _staRuntime.currentConnectedWifi.rssi );
                    _legacyConfig.status.connected_network_ssid.to_string() =
                        _staRuntime.currentConnectedWifi.ssid.c_str();
                    break;
                case ISta::State::DISCONNECTING:
                    _legacyConfig.status.sta_status = StaStatus::DISCONNECTING;
                    break;
                case ISta::State::DISCONNECTED:
                case ISta::State::UNKNOWN:
                    _legacyConfig.status.sta_status = StaStatus::DISCONNECTED;
                    _legacyConfig.status.connected_network_ssid.to_string().clear();
                    break;
                case ISta::State::ERROR:
                    _legacyConfig.status.sta_status = StaStatus::ERROR;
                    break;
            }
            _legacyConfig.status.config_mode = _managerRuntime.configMode;
        }

        void _syncScannedNetworksToLegacy()
        {
            _legacyConfig.scanned_networks.fill( ScannedNetworkInfo {} );
            for ( uint8_t i = 0; i < ISta::MAX_SCANNED_NETWORKS && i < LegacyWifiConfig::MAX_SCANNED; ++i )
            {
                const WifiConfig & src = _scannedNetworks[ i ];
                if ( src.ssid.empty() )
                    break;
                ScannedNetworkInfo entry {};
                entry.ssid.to_string() = src.ssid.c_str();
                entry.rssi             = static_cast< int16_t >( src.rssi );
                entry.has_password     = src.authMode != AuthMode::OPEN;
                _legacyConfig.scanned_networks.insert_at( i, entry );
            }
        }
    };
}    // namespace AsnPlus::Wifi

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
        explicit WifiManager( LegacyWifiConfig & legacyConfig, uint8_t & activeSlotIndex ) :
            _legacyConfig( legacyConfig ),
            _activeSlotIndex( activeSlotIndex ),
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
            _syncCommandFromLegacy();
            _syncSavedNetworksFromLegacy();
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
        uint8_t &          _activeSlotIndex;

        Manager::Config        _managerConfig {};
        Manager::Request       _request {};
        Manager::Runtime       _managerRuntime {};
        ISta::Runtime          _staRuntime {};
        ISta::ScannedNetworks  _scannedNetworks {};
        Manager::SavedNetworks _savedNetworks {};
        WifiConfig             _bleConnectNetwork {};
        bool                   _bleConnectOverrideActive = false;
        ISta::State            _lastStaStateForLogs      = ISta::State::UNKNOWN;

        Sta     _sta;
        Manager _manager;

        void _syncCommandFromLegacy()
        {
            switch ( _legacyConfig.command )
            {
                case Commands::NO_COMMAND:
                    return;
                case Commands::CONFIG_START:
                    Log::warn( "BLE Wi-Fi command: CONFIG_START" );
                    _request.command = Manager::Command::CONFIG_START;
                    break;
                case Commands::CONFIG_END:
                    Log::warn( "BLE Wi-Fi command: CONFIG_END" );
                    _bleConnectOverrideActive = false;
                    _request.command = Manager::Command::CONFIG_END;
                    break;
                case Commands::SCAN:
                    Log::warn( "BLE Wi-Fi command: SCAN" );
                    _request.command = Manager::Command::SCAN;
                    break;
                case Commands::CONNECT:
                    {
                        _request.command      = Manager::Command::CONNECT;
                        const char * ssid     = reinterpret_cast< const char * >( _legacyConfig.command_data );
                        const char * password = ssid + strlen( ssid ) + 1;
                        Log::warn(
                            "BLE Wi-Fi command: CONNECT ssid='%s' password_len=%u",
                            ssid,
                            static_cast< unsigned >( strlen( password ) )
                        );
                        _selectActiveSlotBySsid( ssid, password );
                        _bleConnectNetwork.ssid.assign( ssid );
                        _bleConnectNetwork.password.assign( password );
                        _bleConnectOverrideActive = true;
                        Log::warn( "BLE CONNECT override enabled: forcing reconnect candidates to ssid='%s'", ssid );
                        WifiConfig   network {};
                        network.ssid.assign( ssid );
                        network.password.assign( password );
                        network.serialize( _request.commandData );
                        break;
                    }
                case Commands::DISCONNECT:
                    Log::warn( "BLE Wi-Fi command: DISCONNECT" );
                    _bleConnectOverrideActive = false;
                    _request.command = Manager::Command::DISCONNECT;
                    break;
                default:
                    Log::warn( "BLE Wi-Fi command: unknown command id=%u", static_cast< unsigned >( _legacyConfig.command ) );
                    return;
            }
            _legacyConfig.command = Commands::NO_COMMAND;
        }

        void _syncSavedNetworksFromLegacy()
        {
            if ( _bleConnectOverrideActive )
            {
                _clearSavedRuntimeCandidates();
                _savedNetworks[ 0 ].ssid.assign( _bleConnectNetwork.ssid.c_str() );
                _savedNetworks[ 0 ].password.assign( _bleConnectNetwork.password.c_str() );
                return;
            }

            if ( _activeSlotIndex < ModuleConfig::Wifi::MAX_SAVED_NETWORKS )
            {
                SavedNetworkInfo & active = _legacyConfig.saved_networks[ _activeSlotIndex ];
                active.ssid.terminate();
                if ( active.ssid.data[ 0 ] != '\0' )
                {
                    _clearSavedRuntimeCandidates();
                    _savedNetworks[ 0 ].ssid.assign( active.ssid.data );
                    _savedNetworks[ 0 ].password.assign( active.password.data );
                    return;
                }
            }

            uint8_t dstIndex = 0;
            if ( _activeSlotIndex < ModuleConfig::Wifi::MAX_SAVED_NETWORKS )
            {
                const SavedNetworkInfo & src = _legacyConfig.saved_networks[ _activeSlotIndex ];
                WifiConfig &             dst = _savedNetworks[ dstIndex++ ];
                dst.ssid.assign( src.ssid.data );
                dst.password.assign( src.password.data );
            }

            for ( uint8_t i = 0; i < ModuleConfig::Wifi::MAX_SAVED_NETWORKS; ++i )
            {
                if ( i == _activeSlotIndex )
                    continue;

                const SavedNetworkInfo & src = _legacyConfig.saved_networks[ i ];
                WifiConfig &             dst = _savedNetworks[ dstIndex++ ];
                dst.ssid.assign( src.ssid.data );
                dst.password.assign( src.password.data );
            }
        }

        void _selectActiveSlotBySsid( const char * targetSsid, const char * requestedPassword )
        {
            if ( targetSsid == nullptr || targetSsid[ 0 ] == '\0' )
            {
                Log::warn( "BLE CONNECT requested with empty SSID" );
                return;
            }

            for ( uint8_t i = 0; i < ModuleConfig::Wifi::MAX_SAVED_NETWORKS; ++i )
            {
                _legacyConfig.saved_networks[ i ].ssid.terminate();
                const char * savedSsid = _legacyConfig.saved_networks[ i ].ssid.data;
                if ( savedSsid[ 0 ] == '\0' )
                    continue;

                if ( strcmp( savedSsid, targetSsid ) != 0 )
                    continue;

                _legacyConfig.saved_networks[ i ].password.terminate();
                const char * savedPassword = _legacyConfig.saved_networks[ i ].password.data;
                const bool passwordsMatch  = requestedPassword != nullptr && strcmp( savedPassword, requestedPassword ) == 0;
                Log::warn(
                    "BLE CONNECT matched slot %u ssid='%s' passwords_match=%s requested_len=%u saved_len=%u",
                    i,
                    targetSsid,
                    passwordsMatch ? "true" : "false",
                    static_cast< unsigned >( requestedPassword ? strlen( requestedPassword ) : 0 ),
                    static_cast< unsigned >( strlen( savedPassword ) )
                );

                if ( _activeSlotIndex != i )
                {
                    Log::info( "Selecting Wi-Fi slot %u for BLE CONNECT to SSID '%s'", i, targetSsid );
                    _activeSlotIndex = i;
                }
                return;
            }

            Log::warn( "BLE CONNECT SSID '%s' was not found in saved slots; active slot remains %u", targetSsid, _activeSlotIndex );
        }

        void _clearSavedRuntimeCandidates()
        {
            for ( uint8_t i = 0; i < ModuleConfig::Wifi::MAX_SAVED_NETWORKS; ++i )
            {
                _savedNetworks[ i ].ssid.assign( "" );
                _savedNetworks[ i ].password.assign( "" );
            }
        }

        void _syncStateToLegacy()
        {
            _logAutoconnectTargetOnStateChange();

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
                    if ( _bleConnectOverrideActive &&
                         _bleConnectNetwork.ssid == _staRuntime.currentConnectedWifi.ssid )
                    {
                        Log::warn(
                            "BLE CONNECT override disabled after successful connect to ssid='%s'",
                            _staRuntime.currentConnectedWifi.ssid.c_str()
                        );
                        _bleConnectOverrideActive = false;
                    }
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

        void _logAutoconnectTargetOnStateChange()
        {
            if ( _staRuntime.state == _lastStaStateForLogs )
                return;

            if ( _staRuntime.state == ISta::State::CONNECTING )
            {
                const char * primarySsid = _savedNetworks[ 0 ].ssid.empty() ? "<empty>" : _savedNetworks[ 0 ].ssid.c_str();
                const char * backupSsid  = _savedNetworks[ 1 ].ssid.empty() ? "<empty>" : _savedNetworks[ 1 ].ssid.c_str();
                Log::warn(
                    "Wi-Fi CONNECTING: active_slot=%u primary_ssid='%s' backup_ssid='%s' ble_override=%s",
                    _activeSlotIndex,
                    primarySsid,
                    backupSsid,
                    _bleConnectOverrideActive ? "true" : "false"
                );
            }

            _lastStaStateForLogs = _staRuntime.state;
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

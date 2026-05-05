#pragma once

#include "program/config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "asn/asn-esp32-wifi/include/netif.hpp"

#include "asn/asn-esp32-wifi/include/https/client.hpp"
#include "asn/asn-esp32-wifi/old/wifi_manager.hpp"

#include "asn/asn-esp32-wifi/include/ethernet/ethernet.hpp"

#include "asn/asn-eg915-driver/include/eg915.hpp"
#include "asn/asn-eg915-driver/include/https/https_client.hpp"

#include "asn/asn-esp32-ble/include/nimble.hpp"
#include "asn/asn-esp32-ble/include/structs.hpp"

#include "components/cloud/request_manager.hpp"
#include "components/mqtt/manager.hpp"

#include "asn/asn-esp32-wifi/include/sntp/sntp.hpp"

#include "database/database.hpp"

#include "structs.hpp"

namespace AsnPlus::Connection
{
    class Manager
    {
    public:
        Manager(
            Config &                 connectionModuleConfig,
            Runtime &                connectionModuleRuntime,
            Database &               database,
            Bluetooth::State &       bluetoothState,
            Bluetooth::Nimble &      nimble,
            Network::W5500Ethernet & ethernet,
            Esp32::Https::Client &   ethClient,
            Wifi::WifiConfig &       wifiConfig,
            Wifi::WifiManager &      wifiManager,
            Esp32::Https::Client &   wifiClient,
            Eg915 &                  lte,
            Eg915HttpsClient &       lteClient,
            Cloud::RequestManager &  requestManager,
            Mqtt::Manager &          mqttManager
        ) :
            _connectionModuleConfig( connectionModuleConfig ),
            _connectionModuleRuntime( connectionModuleRuntime ),
            _database( database ),
            _bluetoothState( bluetoothState ),
            _nimble( nimble ),
            _ethernet( ethernet ),
            _ethClient( ethClient ),
            _wifiConfig( wifiConfig ),
            _wifiManager( wifiManager ),
            _wifiClient( wifiClient ),
            _lte( lte ),
            _lteClient( lteClient ),
            _requestManager( requestManager ),
            _mqttManager( mqttManager )
        {
        }

        bool initialize()
        {
            Log::info( "Initializing" );
            _nimble.initialize();
            _nimble.start();
            _nimble.advertise();

            Esp32::Netif::initialize();

            _ethernet.initialize();
            _ethernet.start();
            _ethClient.initialize();

            _wifiManager.initialize();
            _wifiClient.initialize();

            strncpy(
                _database.networkConfig.wifiConfig.ssid, _wifiConfig.saved_networks[ 0 ].ssid.data, Wifi::SSID_LENGTH
            );
            strncpy(
                _database.networkConfig.wifiConfig.password,
                _wifiConfig.saved_networks[ 0 ].password.data,
                Wifi::PASSWORD_LENGTH
            );

            _lte.initialize();

            // _nbIot.initialize();
            // _nbIotClient.initialize();

            _requestManager.initialize();
            _mqttManager.initialize();

            _sntpManager.initialize();

            Log::info( "Initialized" );
            return true;
        }

        void poll()
        {
            _lte.poll();
            _wifiManager.poll();

            _btStateConversion();
            _ethStatusConversion();
            _wifiStatusConversion();
            _lteStatusConversion();
            _nbIotStateConversion();

            const bool network = isNetworkAvailable();

            if ( network && ! _networkWasAvailable ) _sntpManager.notifyNetworkAvailable();
            _networkWasAvailable = network;
        }

        void httpsPoll()
        {
            if ( ! isNetworkAvailable() ) return;
            _setHttpsClient();
            _requestManager.poll();
        }

        void mqttPoll()
        {
            if ( ! isNetworkAvailable() ) return;
            _mqttManager.poll();
        }

        bool isNetworkAvailable()
        {
            return (
                _connectionModuleRuntime.wifiValue == State::CONNECTED ||
                _connectionModuleRuntime.ethValue == State::CONNECTED
                // ||
                // _connectionModuleRuntime.lteValue == State::CONNECTED ||
                // _connectionModuleRuntime.nbIotValue == State::CONNECTED
            );
        }

    protected:
    private:
        static constexpr const char TAG[]                  = "Connection::Manager";
        using Log                                          = Logger< ProjectConfig::LOG_LEVEL_CONNECTION, TAG >;

        static constexpr uint32_t WIFI_REQUEST_INTERVAL_MS = 60 * 1000;
        static constexpr uint32_t NB_REQUEST_INTERVAL_MS   = 10 * 60 * 1000;

        Config &  _connectionModuleConfig;
        Runtime & _connectionModuleRuntime;

        Database & _database;

        uint64_t            _bluetoothLastChangeTimestamp = 0;
        Bluetooth::State &  _bluetoothState;
        Bluetooth::Nimble & _nimble;

        Network::W5500Ethernet & _ethernet;
        Esp32::Https::Client &   _ethClient;

        Wifi::WifiConfig &  _wifiConfig;
        Wifi::WifiManager & _wifiManager;

        Esp32::Https::Client & _wifiClient;

        Eg915 & _lte;

        Eg915HttpsClient & _lteClient;

        Cloud::RequestManager & _requestManager;
        Mqtt::Manager &         _mqttManager;

        Wifi::Sntp _sntpManager { "pool.ntp.org" };
        bool       _networkWasAvailable = false;

        enum class ActiveTransport : uint8_t
        {
            NONE,
            ETH,
            WIFI,
            LTE
        };
        ActiveTransport _lastActiveTransport = ActiveTransport::NONE;

        bool _lteHttpsTestDone               = false;

        void _btStateConversion()
        {
            if ( _bluetoothState.timestamp == _bluetoothLastChangeTimestamp ) return;
            switch ( _bluetoothState.status )
            {
                case Bluetooth::Status::DISCONNECTED:
                    {
                        _connectionModuleRuntime.btValue = State::DISCONNECTED;
                        break;
                    }
                case Bluetooth::Status::ADVERTISE:
                    {
                        _connectionModuleRuntime.btValue = State::CONNECTING;
                        break;
                    }
                case Bluetooth::Status::CONNECTED:
                    {
                        _connectionModuleRuntime.btValue = State::CONNECTED;
                        break;
                    }
                default:
                    {
                        _connectionModuleRuntime.btValue = State::UNKNOWN;
                        break;
                    }
            }
            _bluetoothLastChangeTimestamp = _bluetoothState.timestamp;
            Log::debug( "BT value: %d", _connectionModuleRuntime.btValue );
        }

        void _ethStatusConversion()
        {
            if ( ! _database.ethStaRuntime.linkUp )
            {
                _connectionModuleRuntime.ethValue = State::DISCONNECTED;
            }
            else if ( _database.ethRuntime.state == Network::W5500Ethernet::State::CONNECTED )
            {
                _connectionModuleRuntime.ethValue = State::CONNECTED;
            }
            else
            {
                _connectionModuleRuntime.ethValue = State::CONNECTING;
            }
            Log::debug( "Ethernet value: %d", _connectionModuleRuntime.ethValue );
        }

        void _wifiStatusConversion()
        {
            switch ( _wifiConfig.status.sta_status )
            {
                case Wifi::StaStatus::DISCONNECTED:
                    {
                        _connectionModuleRuntime.wifiValue = State::DISCONNECTED;
                        break;
                    }
                case Wifi::StaStatus::SCANNING:
                case Wifi::StaStatus::SCAN_DONE:
                case Wifi::StaStatus::CONNECTING:
                    {
                        _connectionModuleRuntime.wifiValue = State::CONNECTING;
                        break;
                    }
                case Wifi::StaStatus::CONNECTED:
                    {
                        _connectionModuleRuntime.wifiValue = State::CONNECTED;
                        break;
                    }
                default:
                    {
                        _connectionModuleRuntime.wifiValue = State::UNKNOWN;
                        break;
                    }
            }
            Log::debug( "Wifi value: %d", _connectionModuleRuntime.wifiValue );
        }

        void _lteStatusConversion()
        {
            switch ( _lte.getStatus() )
            {
                case IModem::Status::CONNECTED:
                    _connectionModuleRuntime.lteValue = State::CONNECTED;
                    break;
                case IModem::Status::CONNECTING:
                    _connectionModuleRuntime.lteValue = State::CONNECTING;
                    break;
                case IModem::Status::DISCONNECTED:
                case IModem::Status::NO_SIM:
                case IModem::Status::UNINITIALIZED:
                    _connectionModuleRuntime.lteValue = State::DISCONNECTED;
                    break;
                default:
                    _connectionModuleRuntime.lteValue = State::UNKNOWN;
                    break;
            }
            Log::debug( "LTE value: %d", _connectionModuleRuntime.lteValue );
        }

        // TODO (DK)
        void _nbIotStateConversion()
        {
            // switch (  )
            // {
            //     case State::DISCONNECTED:
            //         {
            //             _connectionModuleRuntime.nbIotValue = State::DISCONNECTED;
            //             break;
            //         }
            //     case State::CONNECTING:
            //         {
            //             _connectionModuleRuntime.nbIotValue = State::CONNECTING;
            //             break;
            //         }
            //     case State::CONNECTED:
            //         {
            //             _connectionModuleRuntime.nbIotValue = State::CONNECTED;
            //             break;
            //         }
            //     default:
            //         {
            //             _connectionModuleRuntime.nbIotValue = State::UNKNOWN;
            //             break;
            //         }
            // }
            // Log::debug( "NB-IoT value: %d", _connectionModuleRuntime.nbIotValue );
        }

        void _setHttpsClient()
        {
            if ( _connectionModuleRuntime.ethValue == State::CONNECTED )
            {
                if ( _lastActiveTransport != ActiveTransport::ETH )
                {
                    Log::debug( "Setting Ethernet client" );
                    _requestManager.setClient( &_wifiClient );
                    _requestManager.setStateTimerInterval( _database.networkConfig.requestConfig.ethStatusInterval );
                    _lastActiveTransport = ActiveTransport::ETH;
                }
            }
            else if ( _connectionModuleRuntime.wifiValue == State::CONNECTED )
            {
                if ( _lastActiveTransport != ActiveTransport::WIFI )
                {
                    Log::debug( "Setting Wi-Fi client" );
                    _requestManager.setClient( &_wifiClient );
                    _requestManager.setStateTimerInterval( _database.networkConfig.requestConfig.wifiStatusInterval );
                    _lastActiveTransport = ActiveTransport::WIFI;
                }
            }
            // else if ( _connectionModuleRuntime.lteValue == State::CONNECTED )
            // {
            //     if ( _lastActiveTransport != ActiveTransport::LTE )
            //     {
            //         Log::debug( "Setting LTE client" );
            //         // _requestManager.setClient( &_lteClient );
            //         _requestManager.setStateTimerInterval( _database.networkConfig.requestConfig.lteStatusInterval );
            //         _lastActiveTransport = ActiveTransport::LTE;
            //     }
            // }
            // else if ( _connectionModuleRuntime.nbIotValue == State::CONNECTED )
            // {
            //     Log::debug( "Setting NB-IoT client" );
            // }
            else
            {
                Log::debug( "No network connected, setting client to nullptr" );
                _requestManager.setClient( nullptr );
                _lastActiveTransport = ActiveTransport::NONE;
            }
        }
    };
}    // namespace AsnPlus::Connection

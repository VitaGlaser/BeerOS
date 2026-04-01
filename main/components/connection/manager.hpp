#pragma once

#include "program/config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "asn/asn-esp32-wifi/https/client/https_client.hpp"
#include "asn/asn-esp32-wifi/wifi_manager.hpp"

#include "asn/asn-esp32-ble/nimble.hpp"
#include "asn/asn-esp32-ble/structs.hpp"

#include "components/cloud/request_manager.hpp"

#include "components/networking/ethernet.hpp"

#include "program/config.hpp"

#include "structs.hpp"

namespace AsnPlus::Connection
{
    class Manager
    {
    public:
        Manager(
            Config &                    connectionModuleConfig,
            Runtime &                   connectionModuleRuntime,
            Bluetooth::State &          bluetoothState,
            Bluetooth::Nimble &         nimble,
            Networking::W5500Ethernet & ethernet,
            Esp32::Https::Client &      ethClient,
            Wifi::WifiConfig &          wifiConfig,
            Wifi::WifiManager &         wifiManager,
            Esp32::Https::Client &      wifiClient,
            Cloud::RequestManager &     requestManager
        ) :
            _connectionModuleConfig( connectionModuleConfig ),
            _connectionModuleRuntime( connectionModuleRuntime ),
            _bluetoothState( bluetoothState ),
            _nimble( nimble ),
            _ethernet( ethernet ),
            _ethClient( ethClient ),
            _wifiConfig( wifiConfig ),
            _wifiManager( wifiManager ),
            _wifiClient( wifiClient ),
            _requestManager( requestManager )
        {
        }

        bool initialize()
        {
            Log::info( "Initializing" );
            _nimble.initialize();
            _nimble.start();
            _nimble.advertise();

            Networking::Netif::initialize();

            _ethernet.initialize();
            _ethernet.start();
            _ethClient.initialize();

            _wifiManager.initialize();
            _wifiClient.initialize();

            // _lte.initialize();
            // _lteClient.initialize();

            // _nbIot.initialize();
            // _nbIotClient.initialize();

            _requestManager.initialize();
            Log::info( "Initialized" );
            return true;
        }

        void poll()
        {
            Log::debug( "Poll" );
            _wifiManager.poll();

            _btStateConversion();
            _ethStateConversion();
            _wifiStateConversion();
            _lteStateConversion();
            _nbIotStateConversion();

            if ( isNetworkAvailable() )
            {
                _setHttpsClient();
                _requestManager.poll();
            }
        }

        bool isNetworkAvailable()
        {
            return (
                _connectionModuleRuntime.wifiValue == Value::CONNECTED ||
                _connectionModuleRuntime.ethValue == Value::CONNECTED ||
                _connectionModuleRuntime.lteValue == Value::CONNECTED ||
                _connectionModuleRuntime.nbIotValue == Value::CONNECTED
            );
        }

    protected:
    private:
        static constexpr const char TAG[]                  = "Connection::Manager";
        using Log                                          = Logger< ProjectConfig::LOG_LEVEL_CONNECTION, TAG >;

        static constexpr uint32_t WIFI_REQUEST_INTERVAL_MS = 60 * 1000;
        static constexpr uint32_t NB_REQUEST_INTERVAL_MS   = 10 * 60 * 1000;

        Config &                  _connectionModuleConfig;
        Runtime &                 _connectionModuleRuntime;

        Bluetooth::State &  _bluetoothState;
        Bluetooth::Nimble & _nimble;

        Networking::W5500Ethernet & _ethernet;
        Esp32::Https::Client &      _ethClient;

        Wifi::WifiConfig &  _wifiConfig;
        Wifi::WifiManager & _wifiManager;

        Esp32::Https::Client & _wifiClient;

        Cloud::RequestManager & _requestManager;

        void _btStateConversion()
        {
            switch ( _bluetoothState.status )
            {
                case Bluetooth::Status::DISCONNECTED:
                    {
                        _connectionModuleRuntime.btValue = Value::DISCONNECTED;
                        break;
                    }
                case Bluetooth::Status::ADVERTISE:
                    {
                        _connectionModuleRuntime.btValue = Value::CONNECTING;
                        break;
                    }
                case Bluetooth::Status::CONNECTED:
                    {
                        _connectionModuleRuntime.btValue = Value::CONNECTED;
                        break;
                    }
                default:
                    {
                        _connectionModuleRuntime.btValue = Value::UNKNOWN;
                        break;
                    }
            }
            Log::debug( "BT value: %d", _connectionModuleRuntime.btValue );
        }

        // TODO (DK)
        void _ethStateConversion()
        {
            // switch (  )
            // {
            //     case Value::DISCONNECTED:
            //         {
            //             _connectionModuleRuntime.ethValue = Value::DISCONNECTED;
            //             break;
            //         }
            //     case Value::CONNECTING:
            //         {
            //             _connectionModuleRuntime.ethValue = Value::CONNECTING;
            //             break;
            //         }
            //     case Value::CONNECTED:
            //         {
            //             _connectionModuleRuntime.ethValue = Value::CONNECTED;
            //             break;
            //         }
            //     default:
            //         {
            //             _connectionModuleRuntime.ethValue = Value::UNKNOWN;
            //             break;
            //         }
            // }
            // Log::debug( "Ethernet value: %d", _connectionModuleRuntime.ethValue );
        }

        void _wifiStateConversion()
        {
            switch ( _wifiConfig.status.sta_status )
            {
                case Wifi::StaStatus::DISCONNECTED:
                    {
                        _connectionModuleRuntime.wifiValue = Value::DISCONNECTED;
                        break;
                    }
                case Wifi::StaStatus::SCANNING:
                case Wifi::StaStatus::SCAN_DONE:
                case Wifi::StaStatus::CONNECTING:
                    {
                        _connectionModuleRuntime.wifiValue = Value::CONNECTING;
                        break;
                    }
                case Wifi::StaStatus::CONNECTED:
                    {
                        _connectionModuleRuntime.wifiValue = Value::CONNECTED;
                        break;
                    }
                default:
                    {
                        _connectionModuleRuntime.wifiValue = Value::UNKNOWN;
                        break;
                    }
            }
            Log::debug( "Wifi value: %d", _connectionModuleRuntime.wifiValue );
        }

        // TODO (DK)
        void _lteStateConversion()
        {
            // switch (  )
            // {
            //     case Value::DISCONNECTED:
            //         {
            //             _connectionModuleRuntime.lteValue = Value::DISCONNECTED;
            //             break;
            //         }
            //     case Value::CONNECTING:
            //         {
            //             _connectionModuleRuntime.lteValue = Value::CONNECTING;
            //             break;
            //         }
            //     case Value::CONNECTED:
            //         {
            //             _connectionModuleRuntime.lteValue = Value::CONNECTED;
            //             break;
            //         }
            //     default:
            //         {
            //             _connectionModuleRuntime.lteValue = Value::UNKNOWN;
            //             break;
            //         }
            // }
            // Log::debug( "LTE value: %d", _connectionModuleRuntime.lteValue );
        }

        // TODO (DK)
        void _nbIotStateConversion()
        {
            // switch (  )
            // {
            //     case Value::DISCONNECTED:
            //         {
            //             _connectionModuleRuntime.nbIotValue = Value::DISCONNECTED;
            //             break;
            //         }
            //     case Value::CONNECTING:
            //         {
            //             _connectionModuleRuntime.nbIotValue = Value::CONNECTING;
            //             break;
            //         }
            //     case Value::CONNECTED:
            //         {
            //             _connectionModuleRuntime.nbIotValue = Value::CONNECTED;
            //             break;
            //         }
            //     default:
            //         {
            //             _connectionModuleRuntime.nbIotValue = Value::UNKNOWN;
            //             break;
            //         }
            // }
            // Log::debug( "NB-IoT value: %d", _connectionModuleRuntime.nbIotValue );
        }

        void _setHttpsClient()
        {
            if ( _connectionModuleRuntime.ethValue == Value::CONNECTED )
            {
                // Set ethernet client
            }
            else if ( _connectionModuleRuntime.wifiValue == Value::CONNECTED )
            {
                _requestManager.setClient( &_wifiClient );
            }
            else if ( _connectionModuleRuntime.lteValue == Value::CONNECTED )
            {
                // Set LTE client
            }
            else if ( _connectionModuleRuntime.nbIotValue == Value::CONNECTED )
            {
                // Set NB-IoT client
            }
            else
            {
                _requestManager.setClient( nullptr );
            }
        }
    };
}    // namespace AsnPlus::Connection

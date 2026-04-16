#pragma once

#include "program/config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "asn/asn-esp32-wifi/netif.hpp"

#include "asn/asn-esp32-wifi/https/client/https_client.hpp"
#include "asn/asn-esp32-wifi/wifi_manager.hpp"

#include "asn/asn-esp32-wifi/ethernet/ethernet.hpp"

#include "asn/asn-eg915-driver/eg915.hpp"
#include "asn/asn-eg915-driver/https/https_client.hpp"

#include "asn/asn-esp32-ble/nimble.hpp"
#include "asn/asn-esp32-ble/structs.hpp"

#include "components/cloud/request_manager.hpp"

#include "asn/asn-esp32-wifi/sntp_manager.hpp"

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
            Cloud::RequestManager &  requestManager
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
            _requestManager( requestManager )
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

            _sntpManager.initialize();

            // TODO: Remove — one-shot HTTPS test
            _lteHttpsTestDone = false;

            Log::info( "Initialized" );
            return true;
        }

        void poll()
        {
            Log::debug( "Poll" );
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

            if ( network )
            {
                _setHttpsClient();
                _requestManager.poll();
            }
        }

        bool isNetworkAvailable()
        {
            return (
                _connectionModuleRuntime.wifiValue == State::CONNECTED ||
                _connectionModuleRuntime.ethValue == State::CONNECTED ||
                _connectionModuleRuntime.lteValue == State::CONNECTED ||
                _connectionModuleRuntime.nbIotValue == State::CONNECTED
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

        Wifi::SntpManager _sntpManager { "pool.ntp.org" };
        bool              _networkWasAvailable = false;

        bool _lteHttpsTestDone                 = false;

        void _btStateConversion()
        {
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
            Log::debug( "BT value: %d", _connectionModuleRuntime.btValue );
        }

        // TODO (DK)
        void _ethStatusConversion()
        {
            // switch (  )
            // {
            //     case State::DISCONNECTED:
            //         {
            //             _connectionModuleRuntime.ethValue = State::DISCONNECTED;
            //             break;
            //         }
            //     case State::CONNECTING:
            //         {
            //             _connectionModuleRuntime.ethValue = State::CONNECTING;
            //             break;
            //         }
            //     case State::CONNECTED:
            //         {
            //             _connectionModuleRuntime.ethValue = State::CONNECTED;
            //             break;
            //         }
            //     default:
            //         {
            //             _connectionModuleRuntime.ethValue = State::UNKNOWN;
            //             break;
            //         }
            // }
            // Log::debug( "Ethernet value: %d", _connectionModuleRuntime.ethValue );
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
            if ( _connectionModuleRuntime.ethValue == State::CONNECTED ||
                 _connectionModuleRuntime.wifiValue == State::CONNECTED )
            {
                _requestManager.setClient( &_wifiClient );
            }
            else if ( _connectionModuleRuntime.lteValue == State::CONNECTED )
            {
                // Set LTE client
            }
            else if ( _connectionModuleRuntime.nbIotValue == State::CONNECTED )
            {
                // Set NB-IoT client
            }
            else
            {
                _requestManager.setClient( nullptr );
            }
        }

        // TODO: Remove — one-shot HTTPS test
        void _testLteHttps()
        {
            if ( _lteHttpsTestDone ) return;
            if ( _lte.getStatus() != IModem::Status::CONNECTED ) return;

            _lteHttpsTestDone = true;
            Log::info( "LTE connected — running HTTPS test" );

            if ( ! _lteClient.initialize() )
            {
                Log::error( "LTE HTTPS client init failed" );
                return;
            }

            static constexpr const char GET_URL[] =
                "https://europe-north1-level-sensing-development.cloudfunctions.net/test";
            static constexpr const char POST_URL[] =
                "https://europe-north1-level-sensing-development.cloudfunctions.net/testPost";

            static uint8_t response_buf[ 512 ];

            // GET
            {
                Https::Response response {};
                response.response     = response_buf;
                response.responseSize = sizeof( response_buf );

                Https::Request request {};
                request.method  = Https::Method::GET;

                uint16_t status = _lteClient.request( GET_URL, &request, &response );
                Log::temp(
                    "HTTPS GET result: status=%d, body_len=%lu",
                    static_cast< int >( status ),
                    static_cast< unsigned long >( response.responseSize )
                );
                if ( response.responseSize > 0 )
                {
                    response_buf
                        [ response.responseSize < sizeof( response_buf ) ? response.responseSize
                                                                         : sizeof( response_buf ) - 1 ] = '\0';
                    Log::info( "GET response: %s", reinterpret_cast< const char * >( response_buf ) );
                }
            }

            // POST
            {
                static char post_body[] =
                    "{\"device\":\"revosoft-beerhouse\",\"firmware\":\"0.1.0\",\"sensor\":{\"flow\":1.23,\"volume\":456.78},\"status\":\"ok\"}";

                static const Https::HeaderKeyValue post_headers[] = {
                    { "Content-Type", "application/json" },
                };
                static Vector< Https::HeaderKeyValue, 1 > post_headers_vec;
                post_headers_vec.clear();
                post_headers_vec.push_back( post_headers[ 0 ] );

                Https::Response response {};
                response.response     = response_buf;
                response.responseSize = sizeof( response_buf );

                Https::Request request {};
                request.method      = Https::Method::POST;
                request.headers     = &post_headers_vec;
                request.payload     = reinterpret_cast< uint8_t * >( post_body );
                request.payloadSize = static_cast< uint32_t >( sizeof( post_body ) - 1 );

                uint16_t status     = _lteClient.request( POST_URL, &request, &response );
                Log::temp(
                    "HTTPS POST result: status=%d, body_len=%lu",
                    static_cast< int >( status ),
                    static_cast< unsigned long >( response.responseSize )
                );
                if ( response.responseSize > 0 )
                {
                    response_buf
                        [ response.responseSize < sizeof( response_buf ) ? response.responseSize
                                                                         : sizeof( response_buf ) - 1 ] = '\0';
                    Log::info( "POST response: %s", reinterpret_cast< const char * >( response_buf ) );
                }
            }
        }
    };
}    // namespace AsnPlus::Connection

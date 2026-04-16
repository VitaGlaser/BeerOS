#pragma once

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/timer.hpp"
#include "asn/asn-core/vector.hpp"

#include "asn/asn-esp32-wifi/https/client/https_client.hpp"

#include "database/database.hpp"

#include "requests/channel_config_request.hpp"
#include "requests/channel_event_request.hpp"
#include "requests/connection_config_request.hpp"
#include "requests/device_config_request.hpp"
#include "requests/object_get_request.hpp"
#include "requests/object_post_request.hpp"
#include "requests/state_post_request.hpp"
#include "requests/time_config_request.hpp"

#include "asn/asn-hal/common/common_structs.hpp"

#include "json_conversion/state_response.hpp"
#include "structs.hpp"

namespace AsnPlus::Cloud
{
    class RequestManager
    {
    public:
        RequestManager( Database & database ) : _database( database ) {}

        bool initialize()
        {
            Log::info( "Initializing" );

            _environment = _database.manufactureInfo.environment;
            _reloadEnvironment();

            if ( ! _statePostRequest.initialize() )
            {
                Log::error( "Failed to initialize statePostRequest" );
                return false;
            }

            if ( ! _timeConfigRequest.initialize() )
            {
                Log::error( "Failed to initialize timeConfigRequest" );
                return false;
            }

            if ( ! _deviceConfigRequest.initialize() )
            {
                Log::error( "Failed to initialize deviceConfigRequest" );
                return false;
            }

            if ( ! _networkConfigRequest.initialize() )
            {
                Log::error( "Failed to initialize networkConfigRequest" );
                return false;
            }

            for ( uint8_t i = 0; i < 4; ++i )
            {
                if ( ! _channelConfigRequestPtrs[ i ]->initialize() )
                {
                    Log::error( "Failed to initialize channelConfigRequest[%u]", i );
                    return false;
                }
            }

            for ( uint8_t i = 0; i < 4; ++i )
            {
                if ( ! _channelEventRequestPtrs[ i ]->initialize() )
                {
                    Log::error( "Failed to initialize channelEventRequest[%u]", i );
                    return false;
                }
            }

            Log::info( "Initialized" );
            return true;
        }

        void poll()
        {
            Log::debug( "Poll" );

            if ( _environment == ManufactureInfo::Environment::UNKNOWN )
            {
                Log::info( "Environment is not set" );
                return;
            }

            if ( ! _timer.isElapsed() ) return;
            _timer.start( 60'000 );    // poll every 2 minutes

            _buildStateRequest();

            if ( ! _statePostRequest.send() )    // response updates _stateResponse in-place
            {
                Log::error( "State POST failed — skipping config/event requests" );
                return;
            }

            if ( _stateResponse.timeConfigTimestamp > _database.timeConfig.timestamp ) _timeConfigRequest.send();

            if ( _stateResponse.deviceConfigTimestamp > _database.deviceConfig.timestamp ) _deviceConfigRequest.send();

            if ( _stateResponse.networkConfigTimestamp > _database.networkConfig.timestamp )
                _networkConfigRequest.send();

            for ( uint8_t i = 0; i < 4; ++i )
            {
                if ( _stateResponse.channelConfigTimestamps[ i ] > _database.channelConfigs[ i ].timestamp )
                    _channelConfigRequestPtrs[ i ]->send();
            }

            for ( uint8_t i = 0; i < 4; ++i )
            {
                _channelEventRequestPtrs[ i ]->send();
            }
        }

        void setStateResponse( const StateResponse & response ) { _stateResponse = response; }

        void setClient( Esp32::Https::IClient * client )
        {
            Log::debug( "Setting client" );
            _client = client;

            _statePostRequest.setClient( client );
            _timeConfigRequest.setClient( client );
            _deviceConfigRequest.setClient( client );
            _networkConfigRequest.setClient( client );

            for ( uint8_t i = 0; i < 4; ++i )
            {
                _channelConfigRequestPtrs[ i ]->setClient( client );
            }

            for ( uint8_t i = 0; i < 4; ++i )
            {
                _channelEventRequestPtrs[ i ]->setClient( client );
            }
        }

    private:
        static constexpr const char TAG[]     = "RequestManager";
        using Log                             = Logger< ProjectConfig::LOG_LEVEL_CLOUD, TAG >;

        static constexpr uint16_t BUFFER_SIZE = 2048;

        // ─── Infrastructure ───────────────────────────────────────────────────

        Database &                                    _database;
        ManufactureInfo::ManufactureInfo::Environment _environment =
            ManufactureInfo::ManufactureInfo::Environment::UNKNOWN;
        Esp32::Https::IClient * _client = nullptr;

        Vector< uint8_t, BUFFER_SIZE > _buffer {};
        Vector< uint8_t, BUFFER_SIZE > _responseBuffer {};

        Timer<> _timer {};

        StateResponse _stateResponse {};
        StateRequest  _stateRequestData {};

        // ─── Request configs ──────────────────────────────────────────────────

        Esp32::Https::IFirestoreRequest::Config _statePostRequestConfig {
            Esp32::Https::IClient::Method::POST,
            _database.manufactureInfo.uid,
            "",
            MOCK_STATE_URL,
            0
        };

        StatePostRequest _statePostRequest {
            _stateRequestData,
            _stateResponse,
            _statePostRequestConfig,
            _responseBuffer,
            Delegate< void() >::create< RequestManager, &RequestManager::_onStateResponse >( *this )
        };

        Esp32::Https::IFirestoreRequest::Config _timeConfigRequestConfig {
            Esp32::Https::IClient::Method::GET,
            _database.manufactureInfo.uid,
            "",
            MOCK_TIME_CONFIG_URL,
            0
        };

        TimeConfigRequest _timeConfigRequest {
            _database.timeConfig,
            _timeConfigRequestConfig,
            _responseBuffer,
            Delegate< void() >::create< RequestManager, &RequestManager::_onTimeConfigUpdate >( *this )
        };

        Esp32::Https::IFirestoreRequest::Config _deviceConfigRequestConfig {
            Esp32::Https::IClient::Method::GET,
            _database.manufactureInfo.uid,
            "",
            MOCK_DEVICE_CONFIG_URL,
            0
        };

        DeviceConfigRequest _deviceConfigRequest {
            _database.deviceConfig,
            _deviceConfigRequestConfig,
            _responseBuffer,
            Delegate< void() >::create< RequestManager, &RequestManager::_onDeviceConfigUpdate >( *this )
        };

        Esp32::Https::IFirestoreRequest::Config _networkConfigRequestConfig {
            Esp32::Https::IClient::Method::GET,
            _database.manufactureInfo.uid,
            "",
            MOCK_CONNECTION_CONFIG_URL,
            0
        };

        NetworkConfigRequest _networkConfigRequest {
            _database.networkConfig,
            _networkConfigRequestConfig,
            _responseBuffer,
            Delegate< void() >::create< RequestManager, &RequestManager::_onNetworkConfigUpdate >( *this )
        };

        Array< Esp32::Https::IFirestoreRequest::Config, 4 > _channelConfigRequestConfigs {
            {
             { Esp32::Https::IClient::Method::GET, _database.manufactureInfo.uid, "", MOCK_CHANNEL_CONFIG_URL, 1 },
             { Esp32::Https::IClient::Method::GET, _database.manufactureInfo.uid, "", MOCK_CHANNEL_CONFIG_URL, 2 },
             { Esp32::Https::IClient::Method::GET, _database.manufactureInfo.uid, "", MOCK_CHANNEL_CONFIG_URL, 3 },
             { Esp32::Https::IClient::Method::GET, _database.manufactureInfo.uid, "", MOCK_CHANNEL_CONFIG_URL, 4 },
             }
        };

        ChannelConfigRequest _channelConfigRequest0 {
            _database.channelConfigs[ 0 ],
            _channelConfigRequestConfigs[ 0 ],
            _responseBuffer,
            Delegate< void() >::create< RequestManager, &RequestManager::_onChannelConfigUpdate< 1 > >( *this )
        };

        ChannelConfigRequest _channelConfigRequest1 {
            _database.channelConfigs[ 1 ],
            _channelConfigRequestConfigs[ 1 ],
            _responseBuffer,
            Delegate< void() >::create< RequestManager, &RequestManager::_onChannelConfigUpdate< 2 > >( *this )
        };

        ChannelConfigRequest _channelConfigRequest2 {
            _database.channelConfigs[ 2 ],
            _channelConfigRequestConfigs[ 2 ],
            _responseBuffer,
            Delegate< void() >::create< RequestManager, &RequestManager::_onChannelConfigUpdate< 3 > >( *this )
        };

        ChannelConfigRequest _channelConfigRequest3 {
            _database.channelConfigs[ 3 ],
            _channelConfigRequestConfigs[ 3 ],
            _responseBuffer,
            Delegate< void() >::create< RequestManager, &RequestManager::_onChannelConfigUpdate< 4 > >( *this )
        };

        Array< ChannelConfigRequest *, 4 > _channelConfigRequestPtrs {
            &_channelConfigRequest0,
            &_channelConfigRequest1,
            &_channelConfigRequest2,
            &_channelConfigRequest3
        };

        Array< Esp32::Https::IFirestoreRequest::Config, 4 > _channelEventRequestConfigs {
            {
             { Esp32::Https::IClient::Method::POST, _database.manufactureInfo.uid, "", MOCK_CHANNEL_HISTORY_URL, 1 },
             { Esp32::Https::IClient::Method::POST, _database.manufactureInfo.uid, "", MOCK_CHANNEL_HISTORY_URL, 2 },
             { Esp32::Https::IClient::Method::POST, _database.manufactureInfo.uid, "", MOCK_CHANNEL_HISTORY_URL, 3 },
             { Esp32::Https::IClient::Method::POST, _database.manufactureInfo.uid, "", MOCK_CHANNEL_HISTORY_URL, 4 },
             }
        };

        ChannelEventRequest _channelEventRequest0 {
            _database.eventHistory0,
            _channelEventRequestConfigs[ 0 ],
            _responseBuffer
        };

        ChannelEventRequest _channelEventRequest1 {
            _database.eventHistory1,
            _channelEventRequestConfigs[ 1 ],
            _responseBuffer
        };

        ChannelEventRequest _channelEventRequest2 {
            _database.eventHistory2,
            _channelEventRequestConfigs[ 2 ],
            _responseBuffer
        };

        ChannelEventRequest _channelEventRequest3 {
            _database.eventHistory3,
            _channelEventRequestConfigs[ 3 ],
            _responseBuffer
        };

        Array< ChannelEventRequest *, 4 > _channelEventRequestPtrs {
            &_channelEventRequest0,
            &_channelEventRequest1,
            &_channelEventRequest2,
            &_channelEventRequest3
        };

        // ─── Callbacks ────────────────────────────────────────────────────────

        void _onStateResponse() { Log::info( "StateResponse received (timestamp: %llu)", _stateResponse.timestamp ); }

        void _onTimeConfigUpdate()
        {
            Log::info( "TimeConfig updated (timestamp: %llu)", _database.timeConfig.timestamp );
        }

        void _onDeviceConfigUpdate()
        {
            Log::info( "DeviceConfig updated (timestamp: %llu)", _database.deviceConfig.timestamp );
            _database.saveDeviceConfig();
        }

        void _onNetworkConfigUpdate()
        {
            Log::info( "NetworkConfig updated (timestamp: %llu)", _database.networkConfig.timestamp );
            _database.saveNetworkConfig();
        }

        template< uint8_t CHANNEL >
        void _onChannelConfigUpdate()
        {
            Log::info(
                "ChannelConfig[%u] updated (timestamp: %llu)", CHANNEL, _database.channelConfigs[ CHANNEL ].timestamp
            );
            _database.saveChannelConfig( CHANNEL - 1 );
        }

        // ─── Helpers ──────────────────────────────────────────────────────────

        void _buildStateRequest()
        {
            _stateRequestData.timestamp = _database.timeRuntime.utcEpochMs;
            _stateRequestData.status    = AsnPlus::Status::OK;

            auto & conn                 = _stateRequestData.connectionState;
            conn.timestamp              = _database.timeRuntime.utcEpochMs;
            conn.ethStatus              = _database.connectionModuleRuntime.ethValue;
            conn.wifiStatus             = _database.connectionModuleRuntime.wifiValue;
            conn.lteStatus              = _database.connectionModuleRuntime.lteValue;

            for ( uint8_t i = 0; i < StateRequest::CHANNEL_COUNT; ++i )
            {
                auto & ch      = _stateRequestData.channelsState[ i ];
                auto & runtime = _database.channelRuntimes[ i ];

                ch.temperature = static_cast< uint64_t >( runtime.temperature );
                ch.pressure    = static_cast< uint64_t >( runtime.pressure );
            }
        }

        static const char * getBaseUrl( ManufactureInfo::Environment env )
        {
            switch ( env )
            {
                case ManufactureInfo::Environment::DEVELOP:
                    Log::info( "Using DEVELOP environment: %s", DEVELOP_URL_BASE );
                    return DEVELOP_URL_BASE;
                case ManufactureInfo::Environment::STAGING:
                    Log::info( "Using STAGING environment: %s", STAGING_URL_BASE );
                    return STAGING_URL_BASE;
                case ManufactureInfo::Environment::PRODUCTION:
                    Log::info( "Using PRODUCTION environment: %s", PRODUCTION_URL_BASE );
                    return PRODUCTION_URL_BASE;
                default:
                    Log::error( "Unknown environment: %d", static_cast< int >( env ) );
                    return "";
            }
        }

        void _reloadEnvironment()
        {
            const char * baseUrl                = getBaseUrl( _environment );

            _statePostRequestConfig.baseUrl     = baseUrl;
            _timeConfigRequestConfig.baseUrl    = baseUrl;
            _deviceConfigRequestConfig.baseUrl  = baseUrl;
            _networkConfigRequestConfig.baseUrl = baseUrl;

            for ( auto & cfg : _channelConfigRequestConfigs )
            {
                cfg.baseUrl = baseUrl;
            }

            for ( auto & cfg : _channelEventRequestConfigs )
            {
                cfg.baseUrl = baseUrl;
            }
        }
    };
}    // namespace AsnPlus::Cloud

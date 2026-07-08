#pragma once

#include <cstring>

#include "freertos/FreeRTOS.h"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/timer.hpp"
#include "asn/asn-core/vector.hpp"

#include "asn/asn-esp32-wifi/include/https/client.hpp"

#include "database/database.hpp"

#include "requests/channel_config_request.hpp"
#include "requests/channel_event_request.hpp"
#include "requests/config_upload_request.hpp"
#include "requests/connection_config_request.hpp"
#include "requests/device_config_request.hpp"
#include "requests/mqtt_config_request.hpp"
#include "requests/object_get_request.hpp"
#include "requests/object_post_request.hpp"
#include "requests/state_post_request.hpp"
#include "requests/time_config_request.hpp"
#include "requests/unit_status_request.hpp"

#include "asn/asn-hal/include/common/common_structs.hpp"

#include "json_conversion/state_response.hpp"
#include "structs.hpp"

namespace AsnPlus::Cloud
{
    class RequestManager
    {
    public:
        struct OtaJob
        {
            bool mandatory = false;
            char version[ 32 ] {};
            char url[ 192 ] {};
        };

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

            if ( ! _timeConfigGetRequest.initialize() )
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

            if ( ! _mqttConfigGetRequest.initialize() )
            {
                Log::error( "Failed to initialize mqttConfigGetRequest" );
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

            for ( uint8_t i = 0; i < 4; ++i )
            {
                if ( ! _channelProfileWebhookRequestPtrs[ i ]->initialize() )
                {
                    Log::error( "Failed to initialize channelProfileWebhookRequest[%u]", i );
                    return false;
                }
            }

            if ( ! _timeConfigPostRequest.initialize() )
            {
                Log::error( "Failed to initialize timeConfigPostRequest" );
                return false;
            }

            if ( ! _deviceConfigPostRequest.initialize() )
            {
                Log::error( "Failed to initialize deviceConfigPostRequest" );
                return false;
            }

            if ( ! _networkConfigPostRequest.initialize() )
            {
                Log::error( "Failed to initialize networkConfigPostRequest" );
                return false;
            }

            if ( ! _mqttConfigPostRequest.initialize() )
            {
                Log::error( "Failed to initialize mqttConfigPostRequest" );
                return false;
            }

            if ( ! _unitStatusRequest.initialize() )
            {
                Log::error( "Failed to initialize unitStatusRequest" );
                return false;
            }

            for ( uint8_t i = 0; i < 4; ++i )
            {
                if ( ! _channelConfigPostRequestPtrs[ i ]->initialize() )
                {
                    Log::error( "Failed to initialize channelConfigPostRequest[%u]", i );
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

            if ( _pollTickIntervalMs == 0 ) return;
            if ( ! _timer.isElapsed() ) return;
            _timer.start( _pollTickIntervalMs );

            if ( _sendStateOnNextTick )
            {
                _buildStateRequest();

                if ( ! _statePostRequest.send() )    // response updates _stateResponse in-place
                {
                    Log::error( "State POST failed — skipping config/event requests" );
                    _sendStateOnNextTick = false;
                    return;
                }

            if ( _onStartup )
            {
                Log::info( "First poll — uploading local configs for server reconciliation" );
                if ( ! _timeConfigPostRequest.send() )
                {
                    Log::error( "Failed to upload time config" );
                    return;
                }
                if ( ! _deviceConfigPostRequest.send() )
                {
                    Log::error( "Failed to upload device config" );
                    return;
                }
                if ( ! _networkConfigPostRequest.send() )
                {
                    Log::error( "Failed to upload network config" );
                    return;
                }
                if ( ! _mqttConfigPostRequest.send() )
                {
                    Log::error( "Failed to upload mqtt config" );
                    return;
                }
                for ( uint8_t i = 0; i < 4; ++i )
                {
                    if ( ! _channelConfigPostRequestPtrs[ i ]->send() )
                    {
                        Log::error( "Failed to upload channel config[%u]", i );
                        return;
                    }
                }
                _onStartup = false;
            }

            if ( _stateResponse.timeConfigTimestamp > _database.timeConfig.timestamp )
                _timeConfigGetRequest.send();
            else if ( _stateResponse.timeConfigTimestamp < _database.timeConfig.timestamp )
                _timeConfigPostRequest.send();

            if ( _stateResponse.deviceConfigTimestamp > _database.deviceConfig.timestamp )
                _deviceConfigRequest.send();
            else if ( _stateResponse.deviceConfigTimestamp < _database.deviceConfig.timestamp )
                _deviceConfigPostRequest.send();

            if ( _stateResponse.networkConfigTimestamp > _database.networkConfig.timestamp )
                _networkConfigRequest.send();
            else if ( _stateResponse.networkConfigTimestamp < _database.networkConfig.timestamp )
                _networkConfigPostRequest.send();

            if ( _stateResponse.mqttConfigTimestamp > _database.mqttConfig.timestamp )
                _mqttConfigGetRequest.send();
            else if ( _stateResponse.mqttConfigTimestamp < _database.mqttConfig.timestamp )
                _mqttConfigPostRequest.send();

            for ( uint8_t i = 0; i < 4; ++i )
            {
                if ( _stateResponse.channelConfigTimestamps[ i ] > _database.channelConfigs[ i ].timestamp )
                    _channelConfigRequestPtrs[ i ]->send();
                else if ( _stateResponse.channelConfigTimestamps[ i ] < _database.channelConfigs[ i ].timestamp )
                    _channelConfigPostRequestPtrs[ i ]->send();
                else
                    Log::debug( "Channel %u config is up to date", i + 1 );
            }

                for ( uint8_t i = 0; i < 4; ++i )
                {
                    _channelEventRequestPtrs[ i ]->send();
                    _channelProfileWebhookRequestPtrs[ i ]->send();
                }

                _sendStateOnNextTick = false;
                return;
            }

            _buildStateRequest();

            if ( ! _unitStatusRequest.send() )
            {
                Log::warn( "Unit status POST failed" );
                _sendStateOnNextTick = true;
                return;
            }
            else if ( _unitStatusRequest.isOtaAvailable() )
            {
                _queueOtaJob(
                    _unitStatusRequest.getOtaVersion(),
                    _unitStatusRequest.getOtaUrl(),
                    _unitStatusRequest.isOtaMandatory()
                );
                Log::warn(
                    "OTA update available: version=%s mandatory=%s url=%s",
                    _unitStatusRequest.getOtaVersion(),
                    _unitStatusRequest.isOtaMandatory() ? "true" : "false",
                    _unitStatusRequest.getOtaUrl()
                );

                // Do not continue with additional cloud requests in the same cycle.
                // OTA task will pick the job and handle update flow.
                _sendStateOnNextTick = true;
                return;
            }

            _sendStateOnNextTick = true;
        }

        void setStateResponse( const StateResponse & response ) { _stateResponse = response; }

        bool tryDequeueOtaJob( OtaJob & job )
        {
            portENTER_CRITICAL( &_otaJobMux );
            if ( ! _otaJobPending )
            {
                portEXIT_CRITICAL( &_otaJobMux );
                return false;
            }
            job            = _otaJob;
            _otaJobPending = false;
            portEXIT_CRITICAL( &_otaJobMux );
            return true;
        }

        bool hasPendingOtaJob()
        {
            portENTER_CRITICAL( &_otaJobMux );
            const bool pending = _otaJobPending;
            portEXIT_CRITICAL( &_otaJobMux );
            return pending;
        }

        void setReportedOtaStatus( const char * status, const char * targetVersion = nullptr )
        {
            _unitStatusRequest.setReportedOtaStatus( status, targetVersion );
        }

        bool sendUnitStatusNow()
        {
            if ( ! _client ) return false;
            _buildStateRequest();
            return _unitStatusRequest.send();
        }

        void setStateTimerInterval( uint32_t interval_ms )
        {
            Log::info( "State timer interval set to %u ms", interval_ms );
            _stateIntervalMs = interval_ms;
            _pollTickIntervalMs = ( _stateIntervalMs > 1 ) ? ( _stateIntervalMs / 2 ) : _stateIntervalMs;
            _sendStateOnNextTick = false;
            _timer.start( _pollTickIntervalMs );
        }

        void setClient( Https::IClient * client )
        {
            Log::debug( "Setting client" );
            _client = client;

            _statePostRequest.setClient( client );
            _timeConfigGetRequest.setClient( client );
            _deviceConfigRequest.setClient( client );
            _networkConfigRequest.setClient( client );

            for ( uint8_t i = 0; i < 4; ++i )
            {
                _channelConfigRequestPtrs[ i ]->setClient( client );
            }

            for ( uint8_t i = 0; i < 4; ++i )
            {
                _channelEventRequestPtrs[ i ]->setClient( client );
                _channelProfileWebhookRequestPtrs[ i ]->setClient( client );
            }

            _timeConfigPostRequest.setClient( client );
            _deviceConfigPostRequest.setClient( client );
            _networkConfigPostRequest.setClient( client );
            _mqttConfigGetRequest.setClient( client );
            _mqttConfigPostRequest.setClient( client );
            _unitStatusRequest.setClient( client );

            for ( uint8_t i = 0; i < 4; ++i )
            {
                _channelConfigPostRequestPtrs[ i ]->setClient( client );
            }
        }

    private:
        static constexpr const char TAG[]     = "RequestManager";
        using Log                             = Logger< ProjectConfig::LOG_LEVEL_CLOUD, TAG >;

        static constexpr uint16_t BUFFER_SIZE = 12288;

        // ─── Infrastructure ───────────────────────────────────────────────────

        Database &                   _database;
        ManufactureInfo::Environment _environment = ManufactureInfo::Environment::UNKNOWN;
        Https::IClient *             _client      = nullptr;
        bool                         _onStartup   = true;

        Vector< uint8_t, BUFFER_SIZE > _buffer {};
        Vector< uint8_t, BUFFER_SIZE > _responseBuffer {};

        Timer<>  _timer {};
        uint32_t _stateIntervalMs = 0;
        uint32_t _pollTickIntervalMs = 0;
        bool     _sendStateOnNextTick = true;

        StateResponse _stateResponse {};
        StateRequest  _stateRequestData {};

        // ─── Request configs ──────────────────────────────────────────────────

        IFirestoreRequest::Config _statePostRequestConfig { nullptr, _database.manufactureInfo.uid, MOCK_STATE_URL, 0 };

        StatePostRequest _statePostRequest {
            _stateRequestData,
            _stateResponse,
            _statePostRequestConfig,
            _buffer,
            _responseBuffer,
            Delegate< void() >::create< RequestManager, &RequestManager::_onStateResponse >( *this )
        };

        IFirestoreRequest::Config
            _timeConfigRequestConfig { nullptr, _database.manufactureInfo.uid, MOCK_TIME_CONFIG_URL, 0 };

        TimeConfigRequest _timeConfigGetRequest {
            _database.timeConfig,
            _timeConfigRequestConfig,
            _responseBuffer,
            Delegate< void() >::create< RequestManager, &RequestManager::_onTimeConfigUpdate >( *this )
        };

        IFirestoreRequest::Config
            _deviceConfigRequestConfig { nullptr, _database.manufactureInfo.uid, MOCK_DEVICE_CONFIG_URL, 0 };

        DeviceConfigRequest _deviceConfigRequest {
            _database.deviceConfig,
            _deviceConfigRequestConfig,
            _responseBuffer,
            Delegate< void() >::create< RequestManager, &RequestManager::_onDeviceConfigUpdate >( *this )
        };

        IFirestoreRequest::Config
            _networkConfigRequestConfig { nullptr, _database.manufactureInfo.uid, MOCK_CONNECTION_CONFIG_URL, 0 };

        NetworkConfigRequest _networkConfigRequest {
            _database.networkConfig,
            _networkConfigRequestConfig,
            _responseBuffer,
            Delegate< void() >::create< RequestManager, &RequestManager::_onNetworkConfigUpdate >( *this )
        };

        IFirestoreRequest::Config
            _mqttConfigRequestConfig { nullptr, _database.manufactureInfo.uid, MOCK_MQTT_CONFIG_URL, 0 };

        MqttConfigRequest _mqttConfigGetRequest {
            _database.mqttConfig,
            _mqttConfigRequestConfig,
            _responseBuffer,
            Delegate< void() >::create< RequestManager, &RequestManager::_onMqttConfigUpdate >( *this )
        };

        Array< IFirestoreRequest::Config, 4 > _channelConfigRequestConfigs {
            {
             { nullptr, _database.manufactureInfo.uid, MOCK_CHANNEL_CONFIG_URL, 0 },
             { nullptr, _database.manufactureInfo.uid, MOCK_CHANNEL_CONFIG_URL, 1 },
             { nullptr, _database.manufactureInfo.uid, MOCK_CHANNEL_CONFIG_URL, 2 },
             { nullptr, _database.manufactureInfo.uid, MOCK_CHANNEL_CONFIG_URL, 3 },
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

        Array< IFirestoreRequest::Config, 4 > _channelEventRequestConfigs {
            {
             { nullptr, _database.manufactureInfo.uid, MOCK_CHANNEL_HISTORY_URL, 0 },
             { nullptr, _database.manufactureInfo.uid, MOCK_CHANNEL_HISTORY_URL, 1 },
             { nullptr, _database.manufactureInfo.uid, MOCK_CHANNEL_HISTORY_URL, 2 },
             { nullptr, _database.manufactureInfo.uid, MOCK_CHANNEL_HISTORY_URL, 3 },
             }
        };

        ChannelEventRequest _channelEventRequest0 {
            _database.eventHistory0,
            _channelEventRequestConfigs[ 0 ],
            _buffer,
            _responseBuffer,
            false,
            true,
            false
        };

        ChannelEventRequest _channelEventRequest1 {
            _database.eventHistory1,
            _channelEventRequestConfigs[ 1 ],
            _buffer,
            _responseBuffer,
            false,
            true,
            false
        };

        ChannelEventRequest _channelEventRequest2 {
            _database.eventHistory2,
            _channelEventRequestConfigs[ 2 ],
            _buffer,
            _responseBuffer,
            false,
            true,
            false
        };

        ChannelEventRequest _channelEventRequest3 {
            _database.eventHistory3,
            _channelEventRequestConfigs[ 3 ],
            _buffer,
            _responseBuffer,
            false,
            true,
            false
        };

        Array< ChannelEventRequest *, 4 > _channelEventRequestPtrs {
            &_channelEventRequest0,
            &_channelEventRequest1,
            &_channelEventRequest2,
            &_channelEventRequest3
        };

        Array< IFirestoreRequest::Config, 4 > _channelProfileWebhookRequestConfigs {
            {
             { PROFILE_WEBHOOK_URL, _database.manufactureInfo.uid, "", 0 },
             { PROFILE_WEBHOOK_URL, _database.manufactureInfo.uid, "", 1 },
             { PROFILE_WEBHOOK_URL, _database.manufactureInfo.uid, "", 2 },
             { PROFILE_WEBHOOK_URL, _database.manufactureInfo.uid, "", 3 },
             }
        };

        ChannelEventRequest _channelProfileWebhookRequest0 {
            _database.eventHistory0,
            _channelProfileWebhookRequestConfigs[ 0 ],
            _buffer,
            _responseBuffer,
            true,
            false,
            true
        };

        ChannelEventRequest _channelProfileWebhookRequest1 {
            _database.eventHistory1,
            _channelProfileWebhookRequestConfigs[ 1 ],
            _buffer,
            _responseBuffer,
            true,
            false,
            true
        };

        ChannelEventRequest _channelProfileWebhookRequest2 {
            _database.eventHistory2,
            _channelProfileWebhookRequestConfigs[ 2 ],
            _buffer,
            _responseBuffer,
            true,
            false,
            true
        };

        ChannelEventRequest _channelProfileWebhookRequest3 {
            _database.eventHistory3,
            _channelProfileWebhookRequestConfigs[ 3 ],
            _buffer,
            _responseBuffer,
            true,
            false,
            true
        };

        Array< ChannelEventRequest *, 4 > _channelProfileWebhookRequestPtrs {
            &_channelProfileWebhookRequest0,
            &_channelProfileWebhookRequest1,
            &_channelProfileWebhookRequest2,
            &_channelProfileWebhookRequest3
        };

        // ─── Upload requests (POST) ───────────────────────────────────────────

        IFirestoreRequest::Config
            _timeConfigPostRequestConfig { nullptr, _database.manufactureInfo.uid, MOCK_TIME_CONFIG_URL, 0 };

        TimeConfigPostRequest
            _timeConfigPostRequest { _database.timeConfig, _timeConfigPostRequestConfig, _buffer, _responseBuffer };

        IFirestoreRequest::Config
            _deviceConfigPostRequestConfig { nullptr, _database.manufactureInfo.uid, MOCK_DEVICE_CONFIG_URL, 0 };

        ConfigUploadRequest< DeviceConfig > _deviceConfigPostRequest {
            _database.deviceConfig,
            _deviceConfigPostRequestConfig,
            _buffer,
            _responseBuffer
        };

        IFirestoreRequest::Config
            _networkConfigPostRequestConfig { nullptr, _database.manufactureInfo.uid, MOCK_CONNECTION_CONFIG_URL, 0 };

        ConfigUploadRequest< NetworkConfig > _networkConfigPostRequest {
            _database.networkConfig,
            _networkConfigPostRequestConfig,
            _buffer,
            _responseBuffer
        };

        IFirestoreRequest::Config
            _mqttConfigPostRequestConfig { nullptr, _database.manufactureInfo.uid, MOCK_MQTT_CONFIG_URL, 0 };

        ConfigUploadRequest< Mqtt::IClient::Config > _mqttConfigPostRequest {
            _database.mqttConfig,
            _mqttConfigPostRequestConfig,
            _buffer,
            _responseBuffer
        };

        IFirestoreRequest::Config
            _unitStatusRequestConfig { UNIT_STATUS_URL, _database.manufactureInfo.uid, "", 0 };

        UnitStatusRequest
            _unitStatusRequest { _stateRequestData, _unitStatusRequestConfig, _buffer, _responseBuffer };

        OtaJob _otaJob {};
        bool   _otaJobPending = false;
        portMUX_TYPE _otaJobMux = portMUX_INITIALIZER_UNLOCKED;

        Array< IFirestoreRequest::Config, 4 > _channelConfigPostConfigs {
            {
             { nullptr, _database.manufactureInfo.uid, MOCK_CHANNEL_CONFIG_URL, 0 },
             { nullptr, _database.manufactureInfo.uid, MOCK_CHANNEL_CONFIG_URL, 1 },
             { nullptr, _database.manufactureInfo.uid, MOCK_CHANNEL_CONFIG_URL, 2 },
             { nullptr, _database.manufactureInfo.uid, MOCK_CHANNEL_CONFIG_URL, 3 },
             }
        };

        ConfigUploadRequest< ChannelConfig, true > _channelConfigPostRequest0 {
            _database.channelConfigs[ 0 ],
            _channelConfigPostConfigs[ 0 ],
            _buffer,
            _responseBuffer
        };

        ConfigUploadRequest< ChannelConfig, true > _channelConfigPostRequest1 {
            _database.channelConfigs[ 1 ],
            _channelConfigPostConfigs[ 1 ],
            _buffer,
            _responseBuffer
        };

        ConfigUploadRequest< ChannelConfig, true > _channelConfigPostRequest2 {
            _database.channelConfigs[ 2 ],
            _channelConfigPostConfigs[ 2 ],
            _buffer,
            _responseBuffer
        };

        ConfigUploadRequest< ChannelConfig, true > _channelConfigPostRequest3 {
            _database.channelConfigs[ 3 ],
            _channelConfigPostConfigs[ 3 ],
            _buffer,
            _responseBuffer
        };

        Array< ConfigUploadRequest< ChannelConfig, true > *, 4 > _channelConfigPostRequestPtrs {
            &_channelConfigPostRequest0,
            &_channelConfigPostRequest1,
            &_channelConfigPostRequest2,
            &_channelConfigPostRequest3
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

        void _onMqttConfigUpdate()
        {
            Log::info( "MqttConfig updated (timestamp: %llu)", _database.mqttConfig.timestamp );
            _database.saveMqttConfig();
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
            _stateRequestData.timestamp                     = _database.timeRuntime.utcEpochMs;
            _stateRequestData.status                        = AsnPlus::Status::OK;
            _stateRequestData.runtime                       = _database.uptime;
            _stateRequestData.batteryVoltage                = _database.batteryVoltage;
            _stateRequestData.chargerAcOk                   = _database.chargerAcOk;
            _stateRequestData.chargerChgOk                  = _database.chargerChgOk;
            _stateRequestData.firmwareInfo.version          = ProjectConfig::fwVersion.NUMBER;
            _stateRequestData.firmwareInfo.dataModelVersion = ProjectConfig::DATA_MODEL_VERSION;
            _stateRequestData.manufactureInfo               = _database.manufactureInfo;

            auto & conn                                     = _stateRequestData.connectionState;
            conn.timestamp                                  = _database.timeRuntime.utcEpochMs;
            conn.ethStatus                                  = _database.connectionModuleRuntime.ethValue;
            conn.wifiStatus                                 = _database.connectionModuleRuntime.wifiValue;
            conn.lteStatus                                  = _database.connectionModuleRuntime.lteValue;

            for ( uint8_t i = 0; i < StateRequest::CHANNEL_COUNT; ++i )
            {
                auto & ch      = _stateRequestData.channelsState[ i ];
                auto & runtime = _database.channelRuntimes[ i ];

                ch.temperature = static_cast< uint64_t >( runtime.temperature );
                ch.pressure    = static_cast< uint64_t >( runtime.pressure );
            }
        }

        void _queueOtaJob( const char * version, const char * url, bool mandatory )
        {
            if ( ! url || url[ 0 ] == '\0' ) return;

            portENTER_CRITICAL( &_otaJobMux );

            _otaJob.mandatory = mandatory;

            if ( version && version[ 0 ] != '\0' )
                StringExt( _otaJob.version, _otaJob.version, sizeof( _otaJob.version ) ).assign( version );
            else
                _otaJob.version[ 0 ] = '\0';

            StringExt( _otaJob.url, _otaJob.url, sizeof( _otaJob.url ) ).assign( url );
            _otaJobPending = true;

            portEXIT_CRITICAL( &_otaJobMux );
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

            _timeConfigPostRequestConfig.baseUrl    = baseUrl;
            _deviceConfigPostRequestConfig.baseUrl  = baseUrl;
            _networkConfigPostRequestConfig.baseUrl = baseUrl;
            _mqttConfigRequestConfig.baseUrl        = baseUrl;
            _mqttConfigPostRequestConfig.baseUrl    = baseUrl;

            for ( auto & cfg : _channelConfigPostConfigs )
            {
                cfg.baseUrl = baseUrl;
            }
        }
    };
}    // namespace AsnPlus::Cloud

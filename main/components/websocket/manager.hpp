#pragma once

#include "program/config.hpp"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "esp_http_server.h"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/utils.hpp"

#include "components/measurement/data_sources/manager.hpp"
#include "database/database.hpp"

namespace AsnPlus::Websocket
{
    class Manager
    {
    public:
        explicit Manager( Database & database ) : _database( database )
        {
            for ( uint8_t channel = 0; channel < CHANNEL_COUNT; ++channel )
            {
                _channelStates[ channel ].lastSeqNum = _database.channelHistorySeqNums[ channel ];
            }
        }

        void setNetworkConnected( bool connected )
        {
            if ( connected == _networkConnected ) return;

            _networkConnected = connected;
            if ( _networkConnected )
            {
                start();
            }
            else
            {
                stop();
            }
        }

        void start()
        {
            if ( _server != nullptr ) return;

            httpd_config_t config = HTTPD_DEFAULT_CONFIG();
            config.uri_match_fn   = httpd_uri_match_wildcard;

            esp_err_t result = httpd_start( &_server, &config );
            if ( result != ESP_OK )
            {
                Log::error( "Failed to start WebSocket server: %s", esp_err_to_name( result ) );
                _server = nullptr;
                return;
            }

            httpd_uri_t wsUri {
                .uri        = "/ws",
                .method     = HTTP_GET,
                .handler    = &Manager::_wsHandler,
                .user_ctx   = this,
                .is_websocket = true,
                .handle_ws_control_frames = false,
                .supported_subprotocol = nullptr,
            };

            result = httpd_register_uri_handler( _server, &wsUri );
            if ( result != ESP_OK )
            {
                Log::error( "Failed to register /ws endpoint: %s", esp_err_to_name( result ) );
                httpd_stop( _server );
                _server = nullptr;
                return;
            }

            _resetClients();
            Log::warn( "WebSocket server started on /ws" );
        }

        void stop()
        {
            if ( _server == nullptr ) return;

            httpd_stop( _server );
            _server = nullptr;
            _resetClients();
            _resetChannelStates();
            Log::info( "WebSocket server stopped" );
        }

        void poll()
        {
            if ( ! _networkConnected || _server == nullptr ) return;

            _pruneDisconnectedClients();
            _sendPendingIdleMessages();
            _broadcastChannelChanges();
        }

    private:
        static constexpr const char TAG[] = "Ws::Manager";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_WEBSOCKET, TAG >;

        static constexpr uint8_t CHANNEL_COUNT = DataSource::Manager::NUM_CHANNELS;
        static constexpr uint8_t MAX_CLIENTS   = 8;

        struct Client
        {
            bool     active            = false;
            int      fd                = -1;
            uint32_t subscriptionMask  = 0;
            bool     pendingIdle       = false;
        };

        struct ChannelState
        {
            uint32_t lastSeqNum        = 0;
            uint32_t activeTapId       = 0;
            uint32_t lastPublishedMl   = UINT32_MAX;
            bool     tapping           = false;
        };

        Database &      _database;
        httpd_handle_t  _server             = nullptr;
        bool            _networkConnected   = false;
        Client          _clients[ MAX_CLIENTS ] {};
        ChannelState    _channelStates[ CHANNEL_COUNT ] {};

        static esp_err_t _wsHandler( httpd_req_t * req )
        {
            auto * manager = static_cast< Manager * >( req->user_ctx );
            if ( manager == nullptr ) return ESP_FAIL;
            return manager->_handleWsRequest( req );
        }

        esp_err_t _handleWsRequest( httpd_req_t * req )
        {
            if ( req->method == HTTP_GET )
            {
                _registerClient( httpd_req_to_sockfd( req ) );
                return ESP_OK;
            }

            httpd_ws_frame_t frame {};
            frame.type = HTTPD_WS_TYPE_TEXT;

            esp_err_t result = httpd_ws_recv_frame( req, &frame, 0 );
            if ( result != ESP_OK )
            {
                Log::warn( "Failed to get WS frame length: %s", esp_err_to_name( result ) );
                return result;
            }

            if ( frame.len == 0 )
            {
                return ESP_OK;
            }

            char payload[ 128 ] {};
            if ( frame.len >= sizeof( payload ) )
            {
                Log::warn( "Ignoring WS frame larger than %u bytes", static_cast< unsigned >( sizeof( payload ) - 1 ) );
                return ESP_OK;
            }

            frame.payload = reinterpret_cast< uint8_t * >( payload );
            result        = httpd_ws_recv_frame( req, &frame, frame.len );
            if ( result != ESP_OK )
            {
                Log::warn( "Failed to read WS frame payload: %s", esp_err_to_name( result ) );
                return result;
            }

            payload[ frame.len ] = '\0';

            const int clientFd = httpd_req_to_sockfd( req );
            _handleClientCommand( clientFd, payload );
            return ESP_OK;
        }

        void _handleClientCommand( int clientFd, char * command )
        {
            Log::info( "WS RX (fd=%d): %s", clientFd, command );

            Client * client = _findClientByFd( clientFd );
            if ( client == nullptr)
            {
                _registerClient( clientFd );
                client = _findClientByFd( clientFd );
                if ( client == nullptr ) return;
            }

            if ( strncmp( command, "sub:", 4 ) == 0 )
            {
                client->subscriptionMask = _parseSubscriptionMask( command + 4 );
                Log::info( "WS SUB (fd=%d): mask=0x%08lx", clientFd, static_cast< unsigned long >( client->subscriptionMask ) );
                return;
            }

            if ( strcmp( command, "get_channels" ) == 0 )
            {
                _sendIdleMessage( clientFd );
                return;
            }

            Log::warn( "WS RX unknown command (fd=%d): %s", clientFd, command );
        }

        void _registerClient( int fd )
        {
            Client * existing = _findClientByFd( fd );
            if ( existing != nullptr )
            {
                existing->active      = true;
                existing->pendingIdle = true;
                return;
            }

            for ( auto & client : _clients )
            {
                if ( client.active ) continue;
                client.active           = true;
                client.fd               = fd;
                client.subscriptionMask = 0;
                client.pendingIdle      = true;
                Log::info( "WS client connected (fd=%d)", fd );
                return;
            }

            Log::warn( "WS client dropped, no free client slot (fd=%d)", fd );
        }

        Client * _findClientByFd( int fd )
        {
            for ( auto & client : _clients )
            {
                if ( client.active && client.fd == fd ) return &client;
            }
            return nullptr;
        }

        void _resetClients()
        {
            for ( auto & client : _clients )
            {
                client = {};
            }
        }

        void _resetChannelStates()
        {
            for ( uint8_t channel = 0; channel < CHANNEL_COUNT; ++channel )
            {
                _channelStates[ channel ] = {};
                _channelStates[ channel ].lastSeqNum = _database.channelHistorySeqNums[ channel ];
            }
        }

        void _pruneDisconnectedClients()
        {
            for ( auto & client : _clients )
            {
                if ( ! client.active ) continue;
                if ( _server == nullptr )
                {
                    client = {};
                    continue;
                }

                if ( httpd_ws_get_fd_info( _server, client.fd ) != HTTPD_WS_CLIENT_WEBSOCKET )
                {
                    Log::info( "WS client disconnected (fd=%d)", client.fd );
                    client = {};
                }
            }
        }

        void _sendPendingIdleMessages()
        {
            for ( auto & client : _clients )
            {
                if ( ! client.active || ! client.pendingIdle ) continue;
                _sendIdleMessage( client.fd );
                client.pendingIdle = false;
            }
        }

        void _broadcastChannelChanges()
        {
            const uint64_t nowMs = _getUtcEpochMs();
            for ( uint8_t channel = 0; channel < CHANNEL_COUNT; ++channel )
            {
                if ( ! _database.channelConfigs[ channel ].enabled ) continue;

                auto &       state         = _channelStates[ channel ];
                const auto & runtime       = _database.channelRuntimes[ channel ];
                const uint32_t currentSeq  = _database.channelHistorySeqNums[ channel ];

                if ( currentSeq != state.lastSeqNum )
                {
                    state.lastSeqNum = currentSeq;
                    state.tapping    = false;
                    state.activeTapId = currentSeq;

                    const uint32_t finalVolumeMl = _getLastEventVolume( channel, currentSeq );
                    state.lastPublishedMl = finalVolumeMl;
                    _broadcastChannelState( channel, state.activeTapId, finalVolumeMl, "finish", nowMs );
                    continue;
                }

                const bool currentlyTapping = ( runtime.flow > 0 ) || ( runtime.volume > 0 );
                if ( ! currentlyTapping )
                {
                    continue;
                }

                if ( ! state.tapping )
                {
                    state.tapping         = true;
                    state.activeTapId     = state.lastSeqNum + 1;
                    state.lastPublishedMl = UINT32_MAX;
                }

                const uint32_t volumeMl = static_cast< uint32_t >( runtime.volume );
                if ( volumeMl == state.lastPublishedMl )
                {
                    continue;
                }

                state.lastPublishedMl = volumeMl;
                _broadcastChannelState( channel, state.activeTapId, volumeMl, "tapping", nowMs );
            }
        }

        void _sendIdleMessage( int fd )
        {
            char channels[ 32 ] {};
            _formatAvailableChannels( channels, sizeof( channels ) );

            char payload[ 256 ] {};
            std::snprintf(
                payload,
                sizeof( payload ),
                "{\"tap_id\":0,\"time\":%llu,\"volume\":0,\"state\":\"idle\",\"available_channels\":%s}",
                _getUtcEpochMs(),
                channels
            );

            _sendToClient( fd, payload );
        }

        void _broadcastChannelState( uint8_t channel, uint32_t tapId, uint32_t volumeMl, const char * state, uint64_t nowMs )
        {
            char payload[ 256 ] {};
            std::snprintf(
                payload,
                sizeof( payload ),
                "{\"channel\":%u,\"tap_id\":%u,\"time\":%llu,\"volume\":%u,\"state\":\"%s\"}",
                channel,
                tapId,
                nowMs,
                volumeMl,
                state
            );

            const uint32_t bit = ( 1u << channel );
            for ( auto & client : _clients )
            {
                if ( ! client.active ) continue;
                if ( ( client.subscriptionMask & bit ) == 0 ) continue;
                _sendToClient( client.fd, payload );
            }
        }

        void _sendToClient( int fd, const char * payload )
        {
            if ( _server == nullptr ) return;

            httpd_ws_frame_t frame {};
            frame.type    = HTTPD_WS_TYPE_TEXT;
            frame.payload = reinterpret_cast< uint8_t * >( const_cast< char * >( payload ) );
            frame.len     = std::strlen( payload );

            esp_err_t result = httpd_ws_send_frame_async( _server, fd, &frame );
            if ( result != ESP_OK )
            {
                Log::warn( "WS send failed (fd=%d): %s", fd, esp_err_to_name( result ) );
                return;
            }

            Log::debug( "WS TX (fd=%d): %s", fd, payload );
        }

        uint32_t _parseSubscriptionMask( char * channelsCsv )
        {
            uint32_t mask = 0;

            char * token = std::strtok( channelsCsv, "," );
            while ( token != nullptr )
            {
                while ( *token != '\0' && std::isspace( static_cast< unsigned char >( *token ) ) )
                {
                    ++token;
                }

                char * end = token + std::strlen( token );
                while ( end > token && std::isspace( static_cast< unsigned char >( *( end - 1 ) ) ) )
                {
                    --end;
                }
                *end = '\0';

                char * parseEnd = nullptr;
                long   channel  = std::strtol( token, &parseEnd, 10 );
                if ( parseEnd != token && *parseEnd == '\0' && channel >= 0 && channel < CHANNEL_COUNT )
                {
                    mask |= ( 1u << static_cast< uint8_t >( channel ) );
                }

                token = std::strtok( nullptr, "," );
            }

            return mask;
        }

        void _formatAvailableChannels( char * out, size_t outSize )
        {
            if ( outSize == 0 ) return;

            size_t used = 0;
            used += std::snprintf( out + used, outSize - used, "[" );

            bool first = true;
            for ( uint8_t channel = 0; channel < CHANNEL_COUNT; ++channel )
            {
                if ( ! _database.channelConfigs[ channel ].enabled ) continue;
                used += std::snprintf( out + used, outSize - used, "%s%u", first ? "" : ",", channel );
                if ( used >= outSize ) break;
                first = false;
            }

            if ( used < outSize )
            {
                std::snprintf( out + used, outSize - used, "]" );
            }
            else
            {
                out[ outSize - 1 ] = '\0';
            }
        }

        uint32_t _getLastEventVolume( uint8_t channel, uint32_t sequenceNumber )
        {
            auto & history = _getHistoryByChannel( channel );
            if ( history.empty() ) return 0;

            const auto & event = history.back();
            if ( static_cast< uint32_t >( event.sequenceNumber ) != sequenceNumber )
            {
                return 0;
            }

            return event.volume;
        }

        IRingBuffer< EventMonitor::Event > & _getHistoryByChannel( uint8_t channel )
        {
            switch ( channel )
            {
                case 0:
                    return _database.eventHistory0;
                case 1:
                    return _database.eventHistory1;
                case 2:
                    return _database.eventHistory2;
                default:
                    return _database.eventHistory3;
            }
        }

        uint64_t _getUtcEpochMs() const
        {
            if ( _database.timeRuntime.utcEpochMs > 0 ) return _database.timeRuntime.utcEpochMs;
            return Utils::getMs64();
        }
    };
}    // namespace AsnPlus::Websocket
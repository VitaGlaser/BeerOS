/**
 * @file manager.hpp
 * @brief Mid-level MQTT manager — publishes per-channel flow values to the broker.
 */
#pragma once

#include "program/config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/span.hpp"
#include "asn/asn-core/string.hpp"
#include "asn/asn-hal/include/common/identification_structs.hpp"
#include "asn/asn-hal/include/mqtt/client.hpp"

#include "database/database.hpp"

namespace AsnPlus::Mqtt
{
    /**
     * @brief Owns flow-value publishing over MQTT.
     *
     *        Call poll() from a dedicated MQTT task.  Call onConnected() from the
     *        IClient::OnConnect delegate to reset publish timestamps on every
     *        (re-)connection so fresh values are sent immediately.
     */
    class Manager
    {
    public:
        Manager( IClient & mqttClient, Database & database ) : _mqttClient( mqttClient ), _database( database ) {}

        bool initialize()
        {
            _lastAppliedTimestamp = 0;
            return true;
        }

        /**
         * @brief Forward from the IClient::OnConnect delegate in Components.
         *        Resets per-channel timestamps so values are re-published immediately.
         */
        void onConnected()
        {
            Log::info( "Connected — republishing all channels" );
            for ( auto & ms : _lastPublishMs ) ms = 0;
            for ( auto & ms : _lastVolumePublishMs ) ms = 0;
            for ( auto & ms : _lastDebugPublishMs ) ms = 0;
        }

        /**
         * @brief Forward from the IClient::OnMessage delegate in Components.
         */
        void onMessage( const IClient::Message & msg )
        {
            Log::debug( "rx: %.*s", static_cast< int >( msg.topic.size() ), msg.topic.data() );
        }

        /**
         * @brief Apply config changes and publish flow values.  Call from the MQTT task
         *        only when the network is available.
         */
        void poll()
        {
            _applyConfigIfChanged();
            _mqttClient.poll();

            _maintainConnection();
            if ( _mqttClient.getState() != IClient::State::CONNECTED ) return;

            _publishFlowData();
            _publishVolumeData();
            _publishFlowDebugInfo();
        }

    private:
        static constexpr const char TAG[]               = "Mqtt::Manager";
        using Log                                       = Logger< ProjectConfig::LOG_LEVEL_MQTT, TAG >;

        static constexpr uint8_t  CHANNEL_COUNT         = 4;
        static constexpr uint32_t HEARTBEAT_INTERVAL_MS = 30'000;
        static constexpr uint32_t RECONNECT_INTERVAL_MS = 15'000;
        static constexpr uint8_t  TOPIC_MAX_LEN         = 64;

        IClient &  _mqttClient;
        Database & _database;

        uint64_t                    _lastAppliedTimestamp = 0;
        uint64_t                    _lastReconnectMs      = 0;
        uint16_t                    _lastPublishedFlow[ CHANNEL_COUNT ] {};
        uint64_t                    _lastPublishMs[ CHANNEL_COUNT ] {};
        uint64_t                    _lastPublishedVolume[ CHANNEL_COUNT ] {};
        uint64_t                    _lastVolumePublishMs[ CHANNEL_COUNT ] {};
        DataSource::Base::DebugInfo _lastPublishedFlowDebug[ CHANNEL_COUNT ] {};
        uint64_t                    _lastDebugPublishMs[ CHANNEL_COUNT ] {};

        void _maintainConnection()
        {
            const IClient::Config & cfg = _database.mqttConfig;
            if ( !cfg.enabled || cfg.brokerUri[ 0 ] == '\0' ) return;

            const auto state = _mqttClient.getState();
            if ( state == IClient::State::CONNECTED || state == IClient::State::CONNECTING ) return;

            const uint64_t now = Utils::getMs64();
            if ( ( now - _lastReconnectMs ) < RECONNECT_INTERVAL_MS ) return;

            _lastReconnectMs = now;
            Log::warn( "MQTT disconnected, reconnecting" );
            _mqttClient.connect();
        }

        void _publishFlowDebugInfo()
        {
            const uint64_t now = Utils::getMs64();

            for ( uint8_t i = 0; i < CHANNEL_COUNT; ++i )
            {
                if ( ! _database.channelConfigs[ i ].enabled ) continue;

                const auto & debug = _database.channelRuntimes[ i ].debugInfoFlow;
                const bool   changed =
                    ( memcmp( debug.data, _lastPublishedFlowDebug[ i ].data, DataSource::Base::DebugInfo::INFO_SIZE ) !=
                      0 );
                const bool timeout = ( ( now - _lastDebugPublishMs[ i ] ) >= HEARTBEAT_INTERVAL_MS );

                if ( ! changed && ! timeout ) continue;

                char topic[ TOPIC_MAX_LEN ];
                snprintf(
                    topic,
                    sizeof( topic ),
                    "%.*s/channel/%u/flow/debug",
                    static_cast< int >( ManufactureInfo::UID_LENGTH ),
                    _database.manufactureInfo.uid,
                    i
                );

                char payload[ DataSource::Base::DebugInfo::INFO_SIZE * 2 + 1 ];
                for ( uint8_t b = 0; b < DataSource::Base::DebugInfo::INFO_SIZE; ++b )
                {
                    snprintf( payload + b * 2, 3, "%02X", debug.data[ b ] );
                }

                _mqttClient.publish(
                    StringView { topic },
                    IConstBytes { reinterpret_cast< const uint8_t * >( payload ), strlen( payload ) }
                );

                _lastPublishedFlowDebug[ i ] = debug;
                _lastDebugPublishMs[ i ]     = now;
            }
        }

        void _applyConfigIfChanged()
        {
            IClient::Config & cfg = _database.mqttConfig;
            if ( cfg.timestamp == _lastAppliedTimestamp ) return;

            _lastAppliedTimestamp = cfg.timestamp;

            if ( cfg.enabled && cfg.brokerUri[ 0 ] != '\0' )
            {
                Log::info( "Config changed — (re)initializing" );

                // Use controlled reconnect attempts from Manager to avoid tight reconnect loops
                // when broker is temporarily unreachable.
                cfg.autoReconnect = false;

                _mqttClient.initialize();
                _lastReconnectMs = Utils::getMs64();
            }
            else
            {
                Log::info( "Config changed — tearing down" );
                _mqttClient.deinit();
            }
        }

        void _publishFlowData()
        {
            const uint64_t now = Utils::getMs64();

            for ( uint8_t i = 0; i < CHANNEL_COUNT; ++i )
            {
                if ( ! _database.channelConfigs[ i ].enabled ) continue;

                const uint16_t flow    = _database.channelRuntimes[ i ].flow;
                const bool     changed = ( flow != _lastPublishedFlow[ i ] );
                const bool     timeout = ( ( now - _lastPublishMs[ i ] ) >= HEARTBEAT_INTERVAL_MS );

                if ( ! changed && ! timeout ) continue;

                char topic[ TOPIC_MAX_LEN ];
                snprintf(
                    topic,
                    sizeof( topic ),
                    "%.*s/channel/%u/flow",
                    static_cast< int >( ManufactureInfo::UID_LENGTH ),
                    _database.manufactureInfo.uid,
                    i
                );

                char payload[ 8 ];
                snprintf( payload, sizeof( payload ), "%u", flow );

                _mqttClient.publish(
                    StringView { topic },
                    IConstBytes { reinterpret_cast< const uint8_t * >( payload ), strlen( payload ) }
                );

                _lastPublishedFlow[ i ] = flow;
                _lastPublishMs[ i ]     = now;
            }
        }

        void _publishVolumeData()
        {
            const uint64_t now = Utils::getMs64();

            for ( uint8_t i = 0; i < CHANNEL_COUNT; ++i )
            {
                if ( ! _database.channelConfigs[ i ].enabled ) continue;

                const uint64_t volume  = _database.channelRuntimes[ i ].volume;
                const bool     changed = ( volume != _lastPublishedVolume[ i ] );
                const bool     timeout = ( ( now - _lastVolumePublishMs[ i ] ) >= HEARTBEAT_INTERVAL_MS );

                if ( ! changed && ! timeout ) continue;

                char topic[ TOPIC_MAX_LEN ];
                snprintf(
                    topic,
                    sizeof( topic ),
                    "%.*s/channel/%u/poured_ml",
                    static_cast< int >( ManufactureInfo::UID_LENGTH ),
                    _database.manufactureInfo.uid,
                    i
                );

                char payload[ 24 ];
                snprintf( payload, sizeof( payload ), "%llu", static_cast< unsigned long long >( volume ) );

                _mqttClient.publish(
                    StringView { topic },
                    IConstBytes { reinterpret_cast< const uint8_t * >( payload ), strlen( payload ) }
                );

                _lastPublishedVolume[ i ] = volume;
                _lastVolumePublishMs[ i ] = now;
            }
        }
    };

}    // namespace AsnPlus::Mqtt

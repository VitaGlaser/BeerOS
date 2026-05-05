/**
 * @file manager.hpp
 * @brief Mid-level MQTT manager — publishes per-channel flow values to the broker.
 */
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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
        Manager( IClient & mqttClient, Database & database ) :
            _mqttClient( mqttClient ),
            _database( database )
        {
        }

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
            memset( _lastPublishTick, 0, sizeof( _lastPublishTick ) );
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
            if ( _mqttClient.getState() != IClient::State::CONNECTED ) return;
            _publishFlowData();
        }

    private:
        static constexpr const char TAG[] = "Mqtt::Manager";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_MQTT, TAG >;

        static constexpr uint8_t  CHANNEL_COUNT         = 4;
        static constexpr uint32_t HEARTBEAT_INTERVAL_MS = 30'000;
        static constexpr uint8_t  TOPIC_MAX_LEN         = 64;

        IClient &  _mqttClient;
        Database & _database;

        uint64_t   _lastAppliedTimestamp               = 0;
        uint16_t   _lastPublishedFlow[ CHANNEL_COUNT ] {};
        TickType_t _lastPublishTick[ CHANNEL_COUNT ]   {};

        void _applyConfigIfChanged()
        {
            const IClient::Config & cfg = _database.mqttConfig;
            if ( cfg.timestamp == _lastAppliedTimestamp ) return;

            _lastAppliedTimestamp = cfg.timestamp;

            if ( cfg.enabled && ! cfg.brokerUri.empty() )
            {
                Log::info( "Config changed — (re)initializing" );
                _mqttClient.initialize();
            }
            else
            {
                Log::info( "Config changed — tearing down" );
                _mqttClient.deinit();
            }
        }

        void _publishFlowData()
        {
            const TickType_t now = xTaskGetTickCount();

            for ( uint8_t i = 0; i < CHANNEL_COUNT; ++i )
            {
                if ( ! _database.channelConfigs[ i ].enabled ) continue;

                const uint16_t flow    = _database.channelRuntimes[ i ].flow;
                const bool     changed = ( flow != _lastPublishedFlow[ i ] );
                const bool     timeout =
                    ( ( now - _lastPublishTick[ i ] ) >= pdMS_TO_TICKS( HEARTBEAT_INTERVAL_MS ) );

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
                _lastPublishTick[ i ]   = now;
            }
        }
    };

}    // namespace AsnPlus::Mqtt

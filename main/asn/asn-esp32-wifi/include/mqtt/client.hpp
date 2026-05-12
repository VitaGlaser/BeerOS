#pragma once

#include "asn_module_config.hpp"

#include "mqtt_client.h"

#include "asn/asn-hal/include/mqtt/client.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/vector.hpp"

namespace AsnPlus::Esp32::Mqtt
{
    class Client : public AsnPlus::Mqtt::IClient
    {
    public:
        // MARK: Types

        using State        = IClient::State;
        using Message      = IClient::Message;
        using Runtime      = IClient::Runtime;
        using OnMessage    = IClient::OnMessage;
        using OnConnect    = IClient::OnConnect;
        using OnDisconnect = IClient::OnDisconnect;

        using Config       = IClient::Config;

        // MARK: Construction

        Client(
            Config &     config,
            Runtime &    runtime,
            OnMessage    onMessage    = {},
            OnConnect    onConnect    = {},
            OnDisconnect onDisconnect = {}
        );

        // MARK: Lifecycle

        bool initialize() override;
        void deinit() override;
        void poll() override;
        void connect() override;
        void disconnect() override;

        // MARK: Operations

        int32_t publish( StringView topic, IConstBytes payload, uint8_t qos = 0, bool retain = false ) override;
        int32_t subscribe( StringView topic, uint8_t qos = 0 ) override;
        int32_t unsubscribe( StringView topic ) override;

    private:
        static constexpr const char TAG[] = "Mqtt::Client";
        using Log                         = Logger< ModuleConfig::Mqtt::LOG_LEVEL, TAG >;

        Config &                 _config;
        esp_mqtt_client_handle_t _handle = nullptr;

        String< ModuleConfig::Mqtt::TOPIC_BUFFER_SIZE >            _topicBuffer;
        Vector< uint8_t, ModuleConfig::Mqtt::PAYLOAD_BUFFER_SIZE > _payloadBuffer;

        void        _destroy();
        static void _eventHandler( void * arg, esp_event_base_t base, int32_t eventId, void * eventData );
        void        _handleEvent( esp_mqtt_event_handle_t event );
    };

}    // namespace AsnPlus::Esp32::Mqtt

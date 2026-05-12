#pragma once

#include "common/common_structs.hpp"

#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/span.hpp"
#include "asn/asn-core/string.hpp"
#include "asn/asn-core/types.hpp"

namespace AsnPlus::Mqtt
{
    /**
     * @brief Platform-independent MQTT client interface.
     *
     *        Derive from this class to provide a platform-specific implementation.
     *        Call initialize() once after the network interface is up, then call
     *        poll() from the main event loop.
     */
    class IClient
    {
    public:
        // MARK: Types

        /**
         * @brief Platform-independent MQTT configuration.
         *        All string fields are owned — safe for runtime writes (e.g. from BLE).
         */
        struct Config : AsnPlus::Config
        {
            static constexpr size_t BROKER_URI_SIZE = 128;
            static constexpr size_t CLIENT_ID_SIZE  = 64;
            static constexpr size_t USERNAME_SIZE   = 64;
            static constexpr size_t PASSWORD_SIZE   = 128;

            bool                      enabled       = true;
            String< BROKER_URI_SIZE > brokerUri;
            String< CLIENT_ID_SIZE >  clientId;
            String< USERNAME_SIZE >   username;
            String< PASSWORD_SIZE >   password;
            uint16_t                  keepalive    = 60;
            bool                      useTls        = false;
            bool                      autoReconnect = true;
        };

        enum class State : uint8_t
        {
            UNKNOWN,
            DISCONNECTED,
            CONNECTING,
            CONNECTED,
            ERROR,
        };

        /**
         * @brief Descriptor passed to the OnMessage delegate on every received message.
         *        All views are valid only for the duration of the callback; copy if needed.
         */
        struct Message
        {
            StringView  topic;
            IConstBytes payload;
            int32_t     msgId;
            uint8_t     qos;
            bool        retain;
        };

        struct Runtime
        {
            State state = State::UNKNOWN;
        };

        using OnMessage    = Delegate< void( const Message & ) >;
        using OnConnect    = Delegate< void() >;
        using OnDisconnect = Delegate< void() >;

        // MARK: Construction

        IClient(
            Runtime &    runtime,
            OnMessage    onMessage    = {},
            OnConnect    onConnect    = {},
            OnDisconnect onDisconnect = {}
        ) :
            _runtime( runtime ),
            _onMessage( onMessage ),
            _onConnect( onConnect ),
            _onDisconnect( onDisconnect )
        {
        }

        // MARK: Lifecycle

        /**
         * @brief Create the client and open the broker connection.
         * @return true on success.
         */
        virtual bool initialize() = 0;

        /**
         * @brief Periodic maintenance tick — call from the main event loop.
         */
        virtual void poll() = 0;

        /** @brief Explicitly open the broker connection (e.g. after disconnect()). */
        virtual void connect() = 0;

        /** @brief Gracefully close the broker connection. */
        virtual void disconnect() = 0;

        /** @brief Destroy the broker connection and release all resources. */
        virtual void deinit() = 0;

        // MARK: Operations

        /**
         * @brief Publish a message to @p topic.
         * @param qos    QoS level: 0, 1, or 2.
         * @param retain Ask the broker to retain the message.
         * @return Assigned message ID on success; -1 on failure.
         */
        virtual int32_t publish( StringView topic, IConstBytes payload, uint8_t qos = 0, bool retain = false ) = 0;

        /**
         * @brief Subscribe to a topic filter.
         * @param qos Maximum QoS for delivered messages.
         * @return Assigned message ID on success; -1 on failure.
         */
        virtual int32_t subscribe( StringView topic, uint8_t qos = 0 ) = 0;

        /**
         * @brief Unsubscribe from a topic filter.
         * @return Assigned message ID on success; -1 on failure.
         */
        virtual int32_t unsubscribe( StringView topic ) = 0;

        State getState() const { return _runtime.state; }

    protected:
        Runtime &    _runtime;
        OnMessage    _onMessage;
        OnConnect    _onConnect;
        OnDisconnect _onDisconnect;
    };

}    // namespace AsnPlus::Mqtt

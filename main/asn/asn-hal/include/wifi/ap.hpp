#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "structs.hpp"

namespace AsnPlus::Wifi
{
    class IAp
    {
    public:
        enum class State : uint8_t
        {
            STARTING,
            STARTED,
            STOPPING,
            STOPPED,
            ERROR
        };

        struct Config
        {
            static constexpr uint8_t MAX_SSID_LENGTH     = 32;
            static constexpr uint8_t MAX_PASSWORD_LENGTH = 64;

            String< MAX_SSID_LENGTH >     ssid;
            String< MAX_PASSWORD_LENGTH > password;

            uint8_t  channel        = 1;
            uint8_t  maxConnections = 4;
            bool     hidden         = false;
            AuthMode authMode       = AuthMode::WPA2;
        };

        struct Runtime
        {
            State   state;
            uint8_t connectedStations = 0;
        };

        IAp( Config & config, Runtime & runtime ) : _config( config ), _runtime( runtime ) {}

        virtual bool initialize() = 0;
        virtual void poll()       = 0;

        virtual void start()      = 0;
        virtual bool stop()       = 0;

    protected:
        Config &  _config;
        Runtime & _runtime;
    };
}    // namespace AsnPlus::Wifi
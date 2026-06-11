#pragma once

#include "asn/asn-core/version.hpp"

namespace AsnPlus
{
    struct ProjectConfig
    {
        static constexpr TVersion< 0, 6, 2 > fwVersion {};
        static constexpr uint32_t            DATA_MODEL_VERSION = 1;

        static constexpr uint8_t LOG_LEVEL                      = 2;
        static constexpr uint8_t LOG_LEVEL_CLOUD                = 2;
        static constexpr uint8_t LOG_LEVEL_CLOUD_REQUESTS       = 2;
        static constexpr uint8_t LOG_LEVEL_CONNECTION           = 2;
        static constexpr uint8_t LOG_LEVEL_ETHERNET             = 2;
        static constexpr uint8_t LOG_LEVEL_MEASUREMENT          = 2;
        static constexpr uint8_t LOG_LEVEL_CHANNEL              = 2;
        static constexpr uint8_t LOG_LEVEL_MODBUS               = 2;
        static constexpr uint8_t LOG_LEVEL_SENSOR               = 2;
        static constexpr uint8_t LOG_LEVEL_DATA_SOURCES         = 2;
        static constexpr uint8_t LOG_LEVEL_MQTT                 = 2;
        static constexpr uint8_t LOG_LEVEL_WEBSOCKET            = 2;
    };
}    // namespace AsnPlus

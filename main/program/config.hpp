#pragma once

#include "asn/asn-core/version.hpp"

namespace AsnPlus
{
    struct ProjectConfig
    {
        static constexpr TVersion< 0, 2, 0 > fwVersion {};

        static constexpr uint8_t LOG_LEVEL                = 1;
        static constexpr uint8_t LOG_LEVEL_CLOUD          = 1;
        static constexpr uint8_t LOG_LEVEL_CLOUD_REQUESTS = 0;
        static constexpr uint8_t LOG_LEVEL_CONNECTION     = 2;
        static constexpr uint8_t LOG_LEVEL_ETHERNET       = 1;
        static constexpr uint8_t LOG_LEVEL_MEASUREMENT    = 1;
        static constexpr uint8_t LOG_LEVEL_MODBUS         = 1;
        static constexpr uint8_t LOG_LEVEL_SENSORS        = 1;
        static constexpr uint8_t LOG_LEVEL_DATA_SOURCES   = 1;
    };
}    // namespace AsnPlus

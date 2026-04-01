#pragma once

#include "stdint.h"

namespace AsnPlus
{
    struct ProgramConfig
    {
        static constexpr bool DEBUG_ENABLED = true;
    };

    struct ModuleConfig
    {
        struct Esp32
        {
            static constexpr uint8_t LOG_LEVEL         = 2;
            static constexpr uint8_t MAX_I2C_DEVICES   = 8;
            static constexpr uint8_t I2C_MAX_READ_WAIT = 5;
        };

        struct Ble
        {
            static constexpr uint8_t LOG_LEVEL         = 2;
            static constexpr uint8_t MAX_SERVICE_COUNT = 8;
            static constexpr bool    PASSWORD_ENABLED  = false;
        };

        struct Wifi
        {
            static constexpr uint8_t LOG_LEVEL = 2;
        };

        struct TimeManager
        {
            static constexpr uint8_t LOG_LEVEL = 1;
        };

        struct Modbus
        {
            static constexpr uint8_t LOG_LEVEL = 2;
        };

        struct Drivers
        {
            static constexpr uint8_t LOG_LEVEL = 2;
        };
    };
}    // namespace AsnPlus

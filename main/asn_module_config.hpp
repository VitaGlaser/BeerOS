#pragma once

#include "asn/asn-core/types.hpp"

namespace AsnPlus
{
    struct ModuleConfig
    {
        struct Hal
        {

            struct Time
            {
                static constexpr uint8_t LOG_LEVEL = 2;
            };

            struct Https
            {
                static constexpr uint8_t LOG_LEVEL    = 2;
                static constexpr uint8_t MAX_HANDLERS = 8;
            };

            struct Modem
            {
                static constexpr uint8_t LOG_LEVEL = 2;
            };
        };

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

        struct Network
        {
            static constexpr uint8_t LOG_LEVEL = 2;
        };

        struct Eg915
        {
            struct At
            {
                static constexpr uint8_t LOG_LEVEL                        = 2;
                static constexpr size_t  URC_PROCESSOR_MAX_HANDLERS_COUNT = 16;
                static constexpr size_t  URC_PROCESSOR_MAX_PREFIX_SIZE    = 16;
                static constexpr size_t  AT_UART_RX_BUFFER_SIZE           = 2048;
                static constexpr size_t  AT_UART_TX_BUFFER_SIZE           = 2048;
                static constexpr size_t  AT_UART_READ_CHUNK_SIZE          = 256;
            };

            static constexpr uint8_t    LOG_LEVEL           = 2;
            static constexpr const char APN[]               = "quectel.vf.std";
            static constexpr bool       USE_RAW_SSL_SOCKETS = true;
            static constexpr size_t     DATA_BUFFER_SIZE    = 2048;
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

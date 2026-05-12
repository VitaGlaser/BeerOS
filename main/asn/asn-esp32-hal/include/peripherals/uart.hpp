// Esp32Uart.hpp
#pragma once

#include "asn/asn-hal/include/peripherals/uart.hpp"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_err.h"

namespace AsnPlus::Esp32
{
    class Uart : public IUart
    {
    public:
        struct Config : public IUart::Config
        {
            uart_port_t port;
            gpio_num_t  txPin;
            gpio_num_t  rxPin;
            int         rxBufferSize = 1024;
            int         txBufferSize = 0;
        };

        Uart( Config & config );

        bool initialize() override;
        int32_t write( const char * data, size_t len ) override;
        int32_t read( char * data, size_t len, uint32_t timeoutMs ) override;

    private:
        Config _config;
    };
}    // namespace AsnPlus::Esp32

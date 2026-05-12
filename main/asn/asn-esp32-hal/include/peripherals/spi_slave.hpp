#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/logger.hpp"

#include "driver/spi_slave.h"

#include "asn/asn-hal/include/peripherals/spi_slave.hpp"

#include "peripherals/gpio.hpp"

namespace AsnPlus::Esp32
{
    class SpiSlave : public ISpiSlave
    {
    public:
        struct Config : public ISpiSlave::Config
        {
            spi_host_device_t host = SPI2_HOST;
            gpio_num_t        sclk;
            gpio_num_t        mosi;
            gpio_num_t        miso;
            gpio_num_t        cs;
        };

        bool initialize() override;

        int32_t write( const uint8_t * data, size_t len, uint32_t timeoutMs ) override;

        int32_t read( uint8_t * data, size_t len, uint32_t timeoutMs ) override;

    protected:
    private:
        static constexpr const char TAG[]       = "SpiSlavez";
        using Log                               = Logger< ModuleConfig::Esp32::LOG_LEVEL, TAG >;

        static constexpr int MESSAGE_QUEUE_SIZE = 5;

        Config _config;

        static void _spiPostSetupCallback( spi_slave_transaction_t * transaction );
        static void _spiPostTransactionCallback( spi_slave_transaction_t * transaction );
    };
}    // namespace AsnPlus::Esp32

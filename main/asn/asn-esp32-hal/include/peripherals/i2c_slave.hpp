#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/logger.hpp"

#include "asn/asn-core/map.hpp"
#include "asn/asn-core/types.hpp"

#include "asn/asn-hal/include/peripherals/i2c_slave.hpp"

#include "driver/gpio.h"
#include "driver/i2c_slave.h"

// TODO: TEST THIS!
namespace AsnPlus::Esp32
{
    class I2CSlave : public II2cSlave
    {
    public:
        struct Config : public II2cSlave::Config
        {
            i2c_port_num_t port = I2C_NUM_0;
            gpio_num_t     sda;
            gpio_num_t     scl;
            bool           pullups = false;
        };

        I2CSlave( Config config ) : II2cSlave( config ), _config( config ) {}

        bool initialize() override;

        int32_t write( const uint8_t * data, size_t len, uint32_t timeoutMs ) override;

        /**
         * NOTE: ESP-IDF doesn't support reading from slave like this, using callback is required
         */
        int32_t read( uint8_t * data, size_t len, uint32_t timeoutMs ) override { return -1; }

    private:
        static constexpr const char TAG[] = "I2cSlave";
        using Log                         = Logger< ModuleConfig::Esp32::LOG_LEVEL, TAG >;

        Config                 _config;
        i2c_slave_dev_handle_t _busHandle = nullptr;

        static bool _i2cOnReceiveCallback(
            i2c_slave_dev_handle_t                 handle,
            const i2c_slave_rx_done_event_data_t * eventData,
            void *                                 arg
        );

        static bool _i2cOnRequestCallback(
            i2c_slave_dev_handle_t                 handle,
            const i2c_slave_request_event_data_t * eventData,
            void *                                 arg
        );
    };
}    // namespace AsnPlus::Esp32

#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/map.hpp"
#include "asn/asn-core/types.hpp"

#include "asn/asn-hal/include/peripherals/i2c_master.hpp"

#include "driver/gpio.h"
#include "driver/i2c_master.h"

namespace AsnPlus::Esp32
{
    class I2cMaster : public II2cMaster
    {
    public:
        struct Config : public II2cMaster::Config
        {
            i2c_port_num_t port = I2C_NUM_0;
            gpio_num_t     sda;
            gpio_num_t     scl;
            bool           pullups = false;
        };

        I2cMaster( Config config ) : II2cMaster( config ), _config( config ) {}

        bool initialize() override;

        int32_t write( uint8_t deviceAddress, const uint8_t * data, size_t len, uint32_t timeoutMs ) override;

        int32_t read( uint8_t deviceAddress, uint8_t * data, size_t len, uint32_t timeoutMs ) override;

        int32_t transfer(
            uint8_t         deviceAddress,
            const uint8_t * writeData,
            size_t          writeLen,
            uint8_t *       readData,
            size_t          readLen,
            uint32_t        timeoutMs
        ) override;

        bool addDevice( uint8_t deviceAddress );

    private:
        static constexpr const char TAG[] = "I2cMaster";
        using Log                         = Logger< ModuleConfig::Esp32::LOG_LEVEL, TAG >;

        Config                                                                        _config;
        i2c_master_bus_handle_t                                                       _busHandle = nullptr;
        Map< uint8_t, i2c_master_dev_handle_t, ModuleConfig::Esp32::MAX_I2C_DEVICES > _devices;

        bool                    _addDeviceToMap( uint8_t deviceAddress, i2c_master_dev_handle_t handle );
        i2c_master_dev_handle_t _findDevice( uint8_t deviceAddress );
    };
}    // namespace AsnPlus::Esp32

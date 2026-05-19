#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/logger.hpp"

#include "driver/spi_master.h"

#include "asn/asn-core/map.hpp"
#include "asn/asn-core/types.hpp"

#include "asn/asn-hal/include/peripherals/spi_master.hpp"

#include "peripherals/gpio.hpp"

namespace AsnPlus::Esp32
{
    class SpiMaster : public ISpiMaster
    {
    public:
        struct Config : public ISpiMaster::Config
        {
            spi_host_device_t host = SPI2_HOST;
            gpio_num_t        sclk;
            gpio_num_t        mosi;
            gpio_num_t        miso;
        };

        SpiMaster( Config & config ) : ISpiMaster( config ), _config( config ) {}

        bool initialize() override;

        int32_t write( uint8_t slaveId, const uint8_t * data, size_t len, uint32_t timeout ) override;

        int32_t read( uint8_t slaveId, uint8_t * data, size_t len, uint32_t timeout ) override;

        int32_t transfer(
            uint8_t         slaveId,
            const uint8_t * writeData,
            size_t          writeLen,
            uint8_t *       readData,
            size_t          readLen,
            uint32_t        timeout
        ) override;

        bool addDevice( uint8_t deviceAddress, IGpio & csPin );

    protected:
        void _selectSlave( uint8_t slaveId ) override;
        void _deselectSlave( uint8_t slaveId ) override;

    private:
        static constexpr const char TAG[]       = "SpiMaster";
        using Log                               = Logger< ModuleConfig::Esp32::LOG_LEVEL, TAG >;

        static constexpr int MESSAGE_QUEUE_SIZE = 5;

        struct Device
        {
            IGpio *             csPin;
            spi_device_handle_t handle;
        };

        Config                                                       _config;
        Map< uint8_t, Device, ModuleConfig::Esp32::MAX_SPI_DEVICES > _devices;

        bool _addDeviceToMap( uint8_t deviceAddress, IGpio & csPin, spi_device_handle_t handle );
        Device * _findDevice( uint8_t deviceAddress );
    };
}    // namespace AsnPlus::Esp32

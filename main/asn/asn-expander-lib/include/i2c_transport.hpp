#pragma once

#include "transport.hpp"
#include "driver/i2c_master.h"
#include <cstdint>

namespace AsnPlus { namespace Esp32 { class I2C; } }

namespace AsnPlus::Expander
{
    class I2cTransport : public Transport
    {
    public:
        I2cTransport( Esp32::I2C & master, uint8_t address, uint32_t frequency );
        void     initialize();
        uint16_t readRegister( uint16_t address ) override;
        void     writeRegister( uint16_t address, uint16_t data ) override;

    private:
        Esp32::I2C &            _master;
        i2c_master_dev_handle_t _device_handle;
        uint8_t                 _address;
        uint32_t                _frequency;
    };
}    // namespace AsnPlus::Expander

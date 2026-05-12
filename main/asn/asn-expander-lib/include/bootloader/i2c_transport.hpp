#pragma once

// #include "bootloader.hpp"
#include "transport.hpp"
#include <cstddef>
#include <cstdint>
#include <sys/_stdint.h>

namespace AsnPlus { namespace Esp32 { class I2C; } }

namespace AsnPlus::Stm32Bootloader
{
    class I2CTransport : public BootloaderTransport
    {
    public:
        I2CTransport( Esp32::I2C & master, uint8_t address );

        void initialize();
        bool synchronizeWithBootloader() override;
        bool startCommand() override;
        bool waitAck() override;
        bool sendData( const uint8_t * data, size_t length ) override;
        bool receiveData( uint8_t * buffer, size_t length ) override;
        TransportType getTransportType() const override;

    private:
        AsnPlus::Esp32::I2C & _i2cMaster;
        uint8_t               _address;
    };
}    // namespace AsnPlus::Stm32Bootloader

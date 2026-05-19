#pragma once

#include "transport.hpp"
#include "asn/asn-hal/include/peripherals/i2c_master.hpp"
#include <cstdint>

namespace AsnPlus::Expander
{
    class I2cTransport : public Transport
    {
    public:
        I2cTransport( II2cMaster & master, uint8_t address );
        void     initialize();
        bool readRegister( uint16_t address, uint16_t & value ) override;
        bool writeRegister( uint16_t address, uint16_t data ) override;
        bool readRegisters( uint16_t startAddress, uint16_t * data, size_t count ) override;
        bool writeRegisters( uint16_t startAddress, const uint16_t * data, size_t count ) override;

        bool synchronizeWithBootlaoder() override;

        bool startCommand() override;
        bool sendData( const uint8_t * data, size_t length ) override;
        bool receiveData( uint8_t * buffer, size_t length )  override;
        bool waitAck() override;

        bool useNoStretchCommands() const override { return true; }

    protected:
        static constexpr uint8_t I2C_BOOTLOADER_ADDRESS = 0x76;
    private:
        II2cMaster & _master;
        uint8_t      _address;
    };
}    // namespace AsnPlus::Expander

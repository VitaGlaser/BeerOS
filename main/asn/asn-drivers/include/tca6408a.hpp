#pragma once

#include "asn/asn-core/types.hpp"
#include "asn/asn-hal/include/peripherals/i2c_master.hpp"

namespace AsnPlus::Drivers
{
    /*
        I2C I/O Expander
        https://www.ti.com/lit/ds/symlink/tca6408a.pdf?ts=1714549530364&ref_url=https%253A%252F%252Fwww.mouser.ch%252F
    */
    class Tca6408a
    {
    public:
        Tca6408a( II2cMaster & i2c, uint8_t address );

        bool initialize( uint8_t ioConfig );
        bool setPin( uint8_t pinNumber );
        bool resetPin( uint8_t pinNumber );
        bool readPin( uint8_t pinNumber );
        bool setOutputPort( uint8_t value );
        bool readInputPort( uint8_t * value );

    private:
        static constexpr uint8_t REG_INPUT_PORT         = 0x00;
        static constexpr uint8_t REG_OUTPUT_PORT        = 0x01;
        static constexpr uint8_t REG_POLARITY_INVERSION = 0x02;
        static constexpr uint8_t REG_CONFIGURATION      = 0x03;

        II2cMaster & _i2c;
        uint8_t      _outputState = 0;
        uint8_t      _address;

        bool _writeByte( uint8_t regAddress, uint8_t data );
        bool _readByte( uint8_t regAddress, uint8_t * data );
    };
}    // namespace AsnPlus::Drivers

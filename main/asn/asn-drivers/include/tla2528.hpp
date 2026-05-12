#pragma once

#include "asn/asn-core/types.hpp"
#include "asn/asn-hal/include/peripherals/i2c_master.hpp"

namespace AsnPlus::Drivers
{
    /*
        12-Bit ADC With I2C Interface and GPIOs
        https://www.ti.com/lit/ds/symlink/tla2528.pdf?ts=1714484307194&ref_url=https%253A%252F%252Fwww.mouser.de%252F
    */
    class Tla2528
    {
    public:
        Tla2528( II2cMaster & i2c, uint8_t address );

        void    initialize();
        int16_t readAdcValue( uint8_t pinNumber );

    private:
        II2cMaster & _i2c;
        uint8_t      _address;

        bool _readInput( uint8_t channel, uint8_t * data, size_t size );
    };
}    // namespace AsnPlus::Drivers

#pragma once

#include "asn/asn-core/axis_values.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-hal/include/peripherals/i2c_master.hpp"

namespace AsnPlus::Drivers
{
    class Tlv493d
    {
    public:
        enum class Mode
        {
            FAST,
            LOWPOWER,
            ULTRA_LOWPOWER,
            MASTER_CONTROLLED_MODE,
            POWER_DOWN_MODE,
        };

        Tlv493d( II2cMaster & i2c, uint8_t address );

        void initialize( Mode mode = Mode::FAST );
        void setMode( Mode mode );
        void temperatureMeasurementEnable( bool enable );
        void readXyz();

        AxisValues< float > getXyz() const;
        float               getX() const;
        float               getY() const;
        float               getZ() const;
        float               getAbsAll() const;

        void parityDisable();

    protected:
        II2cMaster &        _i2c;
        uint8_t             _address         = 0x5E;
        uint8_t             _configRegs[ 4 ] = {};
        AxisValues< float > _xyz;

        void _convertRawToXyz( uint8_t * raw, AxisValues< float > & xyz );
    };
}    // namespace AsnPlus::Drivers

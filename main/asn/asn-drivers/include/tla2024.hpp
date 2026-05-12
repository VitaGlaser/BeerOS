#pragma once

#include "asn/asn-core/types.hpp"
#include "asn/asn-hal/include/peripherals/i2c_master.hpp"

namespace AsnPlus::Drivers
{
    class Tla2024
    {
    public:
        enum OperatingMode
        {
            OP_CONTINUOUS = 0,
            OP_SINGLE     = 1
        };

        enum DataRate
        {
            DR_128SPS  = 0x0,
            DR_250SPS  = 0x1,
            DR_490SPS  = 0x2,
            DR_920SPS  = 0x3,
            DR_1600SPS = 0x4,
            DR_2400SPS = 0x5,
            DR_3300SPS = 0x6,
        };

        enum FullScaleRange
        {
            FSR_6_144V = 0x0,
            FSR_4_096V = 0x1,
            FSR_2_048V = 0x2,
            FSR_1_024V = 0x3,
            FSR_0_512V = 0x4,
            FSR_0_256V = 0x5,
        };

        enum MultiplexerConfig
        {
            MUX_AIN0_AIN1 = 0x0,
            MUX_AIN0_AIN3 = 0x1,
            MUX_AIN1_AIN3 = 0x2,
            MUX_AIN2_AIN3 = 0x3,
            MUX_AIN0_GND  = 0x4,
            MUX_AIN1_GND  = 0x5,
            MUX_AIN2_GND  = 0x6,
            MUX_AIN3_GND  = 0x7,
        };

        Tla2024( II2cMaster & i2c, uint8_t address );

        void  initialize();
        float voltageRead( uint8_t channel );
        float voltageRead();
        bool  setFullScaleRange( FullScaleRange range );
        bool  setMuxConfig( MultiplexerConfig option );
        bool  setOperatingMode( OperatingMode mode );
        bool  setDataRate( DataRate rate );
        bool  reset();
        bool  restore();

    private:
        II2cMaster &   _i2c;
        uint8_t        _address;
        uint8_t        _convReg     = 0x00;
        uint8_t        _confReg     = 0x01;
        uint16_t       _initConf    = 0x8583;
        uint16_t       _savedConf   = 0x8583;
        uint16_t       _maxRawVal   = 0x07FF;
        OperatingMode  _currentMode = OP_CONTINUOUS;
        FullScaleRange _currentFSR  = FSR_2_048V;

        bool    _readRegister( uint8_t reg, uint16_t & value );
        bool    _writeRegister( uint8_t reg, uint16_t value );
        int16_t _analogRead( uint8_t channel );
        int16_t _analogRead();
        float   _currentFullRangeVoltage();
    };
}    // namespace AsnPlus::Drivers

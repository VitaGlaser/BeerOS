#pragma once

#include "asn/asn-hal/include/peripherals/i2c_master.hpp"

namespace AsnPlus::Drivers
{
    // Vishay VEML3235
    // https://www.vishay.com/docs/80131/veml3235.pdf

    class Veml3235
    {
    public:
        static constexpr uint8_t DEVICE_ADDRESS = 0x10;

        Veml3235( II2cMaster & i2c );

        void     initialize();
        void     disable();
        uint16_t readConfig();
        uint16_t readWhite();
        uint16_t readAls();
        uint8_t  readId();

    private:
        enum Reg : uint8_t
        {
            REG_CONFIG     = 0x0,
            REG_DATA_WHITE = 0x4,
            REG_DATA_ALS   = 0x5,
            REG_ID         = 0x9,
        };

        enum Config : uint16_t
        {
            CONFIG_DEFAULT           = 0,

            CONFIG_SHUTDOWN          = 1 << 0,
            CONFIG_TRIGGER           = 1 << 2,
            CONFIG_MANUAL_MODE       = 1 << 3,

            CONFIG_INTEGRATION_50MS  = 0 << 4,
            CONFIG_INTEGRATION_100MS = 1 << 4,
            CONFIG_INTEGRATION_200MS = 2 << 4,
            CONFIG_INTEGRATION_400MS = 3 << 4,
            CONFIG_INTEGRATION_800MS = 4 << 4,

            CONFIG_GAIN_2X           = 1 << 11,
            CONFIG_GAIN_4X           = 3 << 11,

            CONFIG_DIGITAL_GAIN_2X   = 1 << 13,
            CONFIG_SHUTDOWN_CHANNELS = 1 << 15,
        };

        II2cMaster & _i2c;

        void     _writeConfig( uint16_t config );
        uint16_t _read( Reg reg );
    };
}    // namespace AsnPlus::Drivers

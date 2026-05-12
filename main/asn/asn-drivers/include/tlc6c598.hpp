#pragma once

#include "asn/asn-core/types.hpp"
#include "asn/asn-hal/include/peripherals/gpio.hpp"

namespace AsnPlus::Drivers
{
    class Tlc6c598
    {
    public:
        enum class Status
        {
            OK,
        };

        Tlc6c598( IGpio & data, IGpio & clock, IGpio & latch );

        Status initialize();
        void   sendByte( uint8_t value );
        void   clear();

    private:
        IGpio & _data;
        IGpio & _clock;
        IGpio & _latch;

        void _latchPulse();
    };
}    // namespace AsnPlus::Drivers

#pragma once

#include "asn/asn-core/range_mapper.hpp"
#include "asn/asn-core/ring_buffer.hpp"
#include "asn/asn-core/types.hpp"

#include "../peripherals/adc_continuous.hpp"

namespace AsnPlus
{
    class AdcAnalyser
    {
    public:
        struct Config
        {
            uint32_t fullScaleMilliVolts = 3300;
            uint8_t  bits               = 12;
        };

        AdcAnalyser( Config & config, IAdcContinuous::IChannel & channel );

        void poll();

        bool     hasValue() const { return _hasValue; }
        uint32_t getAverageRaw() const { return _raw; }
        uint32_t getAverageMilliVolts() const { return _mv; }

    private:
        Config &                   _config;
        IAdcContinuous::IChannel & _channel;

        uint32_t _raw      = 0;
        uint32_t _mv       = 0;
        bool     _hasValue = false;

        uint32_t _maxCode() const;
        bool     _updateValues( IRingBuffer< uint32_t > & buffer );
    };
}    // namespace AsnPlus

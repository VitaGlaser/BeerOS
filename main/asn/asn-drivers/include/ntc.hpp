#pragma once

#include "asn/asn-core/types.hpp"
#include <cmath>

namespace AsnPlus::Drivers
{
    class Ntc
    {
    public:
        Ntc( float r25, float beta, float pullup );

        float temperature( float voltage );

    private:
        static constexpr float KELVIN_C  = 273.15f;
        static constexpr float SUPPLY_V  = 3.3f;

        float _r25    = 0;
        float _beta   = 0;
        float _pullup = 0;
    };
}    // namespace AsnPlus::Drivers

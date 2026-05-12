#pragma once

#include "asn/asn-core/array.hpp"
#include "asn/asn-core/pixel.hpp"

namespace AsnPlus
{
    class LedStripCommunication
    {
    public:
        virtual bool initialize()                          = 0;
        virtual void poll()                                = 0;
        virtual void setPixel( uint32_t index, Rgb color ) = 0;
        virtual void setPixel( uint32_t index, Hsv color ) = 0;
    };
}    // namespace AsnPlus

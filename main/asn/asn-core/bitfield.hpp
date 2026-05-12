#pragma once

#include "types.hpp"

namespace AsnPlus
{
    struct BitField
    {
        volatile u32 & reg;

        void modify( u8 shift, u32 mask, u32 value ) { reg = ( reg & ~( mask << shift ) ) | ( value << shift ); }

        void write( u32 value ) { reg = value; }

        void toggle( u8 shift, u32 mask ) { reg ^= mask << shift; }

        u32 get( u8 shift, u32 mask ) { return ( reg >> shift ) & mask; }
    };
}    // namespace AsnPlus

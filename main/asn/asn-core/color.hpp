#pragma once

#include "types.hpp"

namespace AsnPlus
{
    struct Color
    {
        u8 red;
        u8 green;
        u8 blue;

        bool operator==(const Color &other) const
        {
            return red == other.red && green == other.green && blue == other.blue;
        }
    };

    namespace Colors
    {
        constexpr Color NONE{0, 0, 0};
        constexpr Color BLACK{0, 0, 0};
        constexpr Color WHITE{255, 255, 255};
        constexpr Color RED{255, 0, 0};
        constexpr Color GREEN{0, 255, 0};
        constexpr Color BLUE{0, 0, 255};
        constexpr Color LIGHT_BLUE{0, 255, 255};
        constexpr Color YELLOW{255, 255, 0};
        constexpr Color PURPLE{255, 0, 255};
        constexpr Color TURQUOISE{0, 255, 255};
        constexpr Color ORANGE{255, 165, 0};
        constexpr Color VIOLET{138, 43, 226};
    };
}
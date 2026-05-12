#pragma once

#include "types.hpp"

namespace AsnPlus
{
    class AnsiCodes
    {
    public:
        static constexpr const char * CLEAR_LINE      = "\033[2K";
        static constexpr const char * CLEAR_SCREEN    = "\033[2J";
        static constexpr const char * HOME            = "\033[H";
        static constexpr const char * HORIZONTAL_HOME = "\033[0G";

        struct Cursor
        {
            static constexpr const char * UP    = "\033[1A";
            static constexpr const char * DOWN  = "\033[1B";
            static constexpr const char * RIGHT = "\033[1C";
            static constexpr const char * LEFT  = "\033[1D";
        };

        struct Style
        {
            static constexpr const char * RESET         = "\033[0m";
            static constexpr const char * BOLD          = "\033[1m";
            static constexpr const char * ITALIC        = "\033[3m";
            static constexpr const char * UNDERLINE     = "\033[4m";
            static constexpr const char * REVERSE       = "\033[7m";
            static constexpr const char * STRIKETHROUGH = "\033[9m";
        };

        struct DummyStyles
        {
            static constexpr const char * RESET         = "";
            static constexpr const char * BOLD          = "";
            static constexpr const char * ITALIC        = "";
            static constexpr const char * UNDERLINE     = "";
            static constexpr const char * REVERSE       = "";
            static constexpr const char * STRIKETHROUGH = "";
        };

        struct Color
        {
            static constexpr const char * RESET   = "\033[0m";
            static constexpr const char * BLACK   = "\033[30m";
            static constexpr const char * RED     = "\033[31m";
            static constexpr const char * GREEN   = "\033[32m";
            static constexpr const char * YELLOW  = "\033[33m";
            static constexpr const char * BLUE    = "\033[34m";
            static constexpr const char * MAGENTA = "\033[35m";
            static constexpr const char * WHITE   = "\033[37m";
        };

        struct DummyColors
        {
            static constexpr const char * RESET   = "";
            static constexpr const char * BLACK   = "";
            static constexpr const char * RED     = "";
            static constexpr const char * GREEN   = "";
            static constexpr const char * YELLOW  = "";
            static constexpr const char * BLUE    = "";
            static constexpr const char * MAGENTA = "";
            static constexpr const char * WHITE   = "";
        };

        struct Background
        {
            static constexpr const char * RESET   = "\033[0m";
            static constexpr const char * BLACK   = "\033[40m";
            static constexpr const char * RED     = "\033[41m";
            static constexpr const char * GREEN   = "\033[42m";
            static constexpr const char * YELLOW  = "\033[43m";
            static constexpr const char * BLUE    = "\033[44m";
            static constexpr const char * MAGENTA = "\033[45m";
            static constexpr const char * WHITE   = "\033[47m";
        };

        struct DummyBackgrounds
        {
            static constexpr const char * RESET   = "";
            static constexpr const char * BLACK   = "";
            static constexpr const char * RED     = "";
            static constexpr const char * GREEN   = "";
            static constexpr const char * YELLOW  = "";
            static constexpr const char * BLUE    = "";
            static constexpr const char * MAGENTA = "";
            static constexpr const char * WHITE   = "";
        };
    };
}    // namespace AsnPlus

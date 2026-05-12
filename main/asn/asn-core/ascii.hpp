#pragma once

#include "types.hpp"

namespace AsnPlus
{
    class Ascii
    {
    public:
        static constexpr u8 NUL = 0,      // null character
            SOH                 = 1,      // start of header
            STX                 = 2,      // start of text
            ETX                 = 3,      // end of text
            EOT                 = 4,      // end of transmission
            ENQ                 = 5,      // enquiry
            ACK                 = 6,      // acknowledge
            BEL                 = 7,      // bell (ring)
            BACKSPACE           = 8,      // backspace
            HORIZONTAL_TAB      = 9,      // horizontal tab
            LINE_FEED           = 10,     // line feed
            VERTICAL_TAB        = 11,     // vertical tab
            FORM_FEED           = 12,     // form feed
            CARRIAGE_RETURN     = 13,     // carriage return
            SO                  = 14,     // shift out
            SI                  = 15,     // shift in
            DLE                 = 16,     // data link escape
            DC1                 = 17,     // device control 1
            DC2                 = 18,     // device control 2
            DC3                 = 19,     // device control 3
            DC4                 = 20,     // device control 4
            NAK                 = 21,     // negative acknowledge
            SYN                 = 22,     // synchronize
            ETB                 = 23,     // end transmission block
            CAN                 = 24,     // cancel
            EM                  = 25,     // end of medium
            SUB                 = 26,     // substitute
            ESC                 = 27,     // escape
            FS                  = 28,     // file separator
            GS                  = 29,     // group separator
            RS                  = 30,     // record separator
            US                  = 31,     // unit separator
            SPACE               = 32,     // space
            DELETE              = 127,    // delete
            MIN_WRITABLE = 32, MAX_WRITABLE = 126;

        static constexpr const char * NEWLINE = "\r\n";

        static bool is_digit( u8 code ) { return code >= '0' && code <= '9'; }

        static bool is_lowercase( u8 code ) { return code >= 'a' && code <= 'z'; }

        static bool is_uppercase( u8 code ) { return code >= 'A' && code <= 'Z'; }

        static bool is_letter( u8 code ) { return is_lowercase( code ) || is_uppercase( code ); }

        static bool is_writable( u8 code ) { return code >= MIN_WRITABLE && code <= MAX_WRITABLE; }
    };
}    // namespace AsnPlus

#pragma once

#include "algs.hpp"

namespace AsnPlus
{
    template< u8 _MAJOR, u8 _MINOR, u16 _PATCH = 0 >
    struct TVersion
    {
        static constexpr u8  MAJOR           = _MAJOR;
        static constexpr u8  MINOR           = _MINOR;
        static constexpr u16 PATCH           = _PATCH;

        // major and minor version numbers combined (major * 100 + minor)
        static constexpr u16 MAJOR_MINOR     = 100 * MAJOR + MINOR;

        // major and minor version numbers combined (major * 256 + minor)
        static constexpr u16 MAJOR_MINOR_BIN = 256 * MAJOR + MINOR;

        // major and minor version numbers combined (major * 100 + minor) and BCD encoded
        static constexpr u16 MAJOR_MINOR_BCD = Alg::bcd_encode( MAJOR_MINOR );

        // major, minor and patch numbers combined (major * 10^6 + minor * 10^4 + patch)
        static constexpr u32 NUMBER          = 10'000 * MAJOR_MINOR + PATCH;

        // major, minor and patch numbers combined (major * 2^24 + minor * 2^16 + patch)
        static constexpr u32 NUMBER_BIN      = 65'536 * MAJOR_MINOR_BIN + PATCH;

        // major, minor and patch numbers combined (major * 10^6 + minor * 10^4 + patch) and BCD encoded
        static constexpr u32 NUMBER_BCD      = Alg::bcd_encode( NUMBER );

        static constexpr bool at_least( u8 major, u8 minor, u16 patch = 0 )
        {
            return NUMBER_BIN >= combine( major, minor, patch );
        }

        static constexpr bool at_most( u8 major, u8 minor, u16 patch = 0 )
        {
            return NUMBER_BIN <= combine( major, minor, patch );
        }

        static constexpr bool equals( u8 major, u8 minor, u16 patch = 0 )
        {
            return NUMBER_BIN == combine( major, minor, patch );
        }

        static constexpr u32 combine( u8 major, u8 minor, u16 patch = 0 )
        {
            return ( major << 24 ) | ( minor << 16 ) | ( patch );
        }
    };

    using CoreVersion = TVersion< 1, 4, 0 >;

    /*
        EXAMPLE:


        struct Build
        {
            // The current firmware version
            using Version = TVersion< 1, 13, 0 >;
        };

        // Prints the current firmware version in the form major.minor.patch
        void print_version()
        {
            printf
            (
                "Firmware version: %hu.%hu.%u\n",
                Build::Version::MAJOR,
                Build::Version::MINOR,
                Build::Version::PATCH
            );
        }

        // Returns the current firmware version as an integer (major * 10^6 + minor * 10^4 + patch)
        u32 get_fw_version()
        {
            return Build::Version::NUMBER;
        }


        class Cli : uCLI::Core
        {
            static_assert( uCLI::Version::at_least( 1, 3 ), "ASN CLI submodule version 1.3 or higher is required." );

            ...
        };

        static_assert( CoreVersion::at_least( 1, 1 ), "ASN Core submodule version 1.1 or higher is required." );
    */
}    // namespace AsnPlus

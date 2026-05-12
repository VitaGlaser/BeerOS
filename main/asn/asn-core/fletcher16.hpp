#pragma once

#include "span.hpp"
#include "types.hpp"

namespace AsnPlus
{
    class Fletcher16
    {
    public:
        u16 msb = 0;
        u16 lsb = 0;

        void reset()
        {
            msb = 0;
            lsb = 0;
        }

        void feed( u8 new_value )
        {
            lsb = ( lsb + new_value ) % 255;
            msb = ( msb + lsb ) % 255;
        }

        void feed( IBytes data )
        {
            for ( auto byte : data )
            {
                feed( byte );
            }
        }

        u16 result() { return msb * 256 + lsb; }

        static u16 compute( IBytes data )
        {
            Fletcher16 instance;

            instance.reset();
            instance.feed( data );

            return instance.result();
        }

        static bool test( IBytes data, u16 expected_checksum ) { return compute( data ) == expected_checksum; }
    };
}    // namespace AsnPlus

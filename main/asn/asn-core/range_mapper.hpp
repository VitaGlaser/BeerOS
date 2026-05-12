#pragma once

#include "algs.hpp"

namespace AsnPlus
{

    struct RangeMapper
    {
        template< typename In, typename Out >
        static constexpr Out map( In input, In from_min, In from_max, Out to_min, Out to_max, bool clamp = false )
        {
            if ( clamp )
            {
                input = etl::clamp( input, etl::min( from_min, from_max ), etl::max( from_min, from_max ) );
            }

            if ( from_min == from_max )
            {
                // avoid division by zero
                return to_min;
            }

            const float ratio = static_cast< float >( input - from_min ) / static_cast< float >( from_max - from_min );
            return static_cast< Out >( to_min + ratio * ( to_max - to_min ) );
        }
    };

}    // namespace AsnPlus

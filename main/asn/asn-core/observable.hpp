#pragma once

#include "flag.hpp"
#include "types.hpp"

namespace AsnPlus
{
    template< typename TYPE >
    struct Observable
    {
        TYPE value {};
        Flag is_changed {};

        void update( TYPE new_value )
        {
            is_changed.set_if( value != new_value );
            value = new_value;
        }
    };
}    // namespace AsnPlus

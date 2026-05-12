#pragma once

#include "etl/type_traits.h"

namespace AsnPlus
{
    template< typename T >
    struct function_info
    {
    };

    template< typename RETURN, typename FIRST, typename... REST >
    struct function_info< RETURN( FIRST, REST... ) >
    {
        using return_type = RETURN;
        using param_type  = FIRST;
    };

    template< typename RETURN, typename CLASS, typename FIRST, typename... REST >
    struct function_info< RETURN ( CLASS::* )( FIRST, REST... ) >
    {
        using return_type = RETURN;
        using param_type  = FIRST;
        using class_type  = CLASS;
    };
}    // namespace AsnPlus

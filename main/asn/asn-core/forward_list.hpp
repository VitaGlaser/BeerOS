#pragma once

#include "etl/forward_list.h"

namespace AsnPlus
{
    template< typename T, const size_t MAX_SIZE >
    using ForwardList = etl::forward_list< T, MAX_SIZE >;

    template< typename T >
    using IForwardList = etl::iforward_list< T >;
}    // namespace AsnPlus

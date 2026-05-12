#pragma once

#include "etl/multiset.h"

namespace AsnPlus
{
    template< typename KEY, typename VALUE, size_t MAX_SIZE, typename COMPARE = etl::less< KEY > >
    using MultiSet = multiset< KEY, VALUE, MAX_SIZE, COMPARE >;

    template< typename KEY, typename VALUE, typename COMPARE = etl::less< KEY > >
    using IMultiSet = imultiset< KEY, VALUE, COMPARE >;
}    // namespace AsnPlus

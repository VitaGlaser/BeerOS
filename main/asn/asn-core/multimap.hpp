#pragma once

#include "etl/multimap.h"

namespace AsnPlus
{
    template< typename KEY, typename VALUE, size_t MAX_SIZE, typename COMPARE = etl::less< KEY > >
    using MultiMap = multimap< KEY, VALUE, MAX_SIZE, COMPARE >;

    template< typename KEY, typename VALUE, typename COMPARE = etl::less< KEY > >
    using IMultiMap = imultimap< KEY, VALUE, COMPARE >;
}    // namespace AsnPlus

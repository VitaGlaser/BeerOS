#pragma once

#include "etl/priority_queue.h"

namespace AsnPlus
{
    template<
        typename T,
        const size_t SIZE,
        typename CONTAINER = etl::vector< T, SIZE >,
        typename COMPARE   = etl::less< typename CONTAINER::value_type > >
    using PriorityQueue = etl::priority_queue< T, SIZE, CONTAINER, COMPARE >;

    template< typename T, typename CONTAINER, typename COMPARE = etl::less< T > >
    using IPriorityQueue = etl::ipriority_queue< T, CONTAINER, COMPARE >;
}    // namespace AsnPlus

#pragma once

#include "etl/queue.h"

namespace AsnPlus
{
    template< typename ITEM_TYPE, const size_t MAX_SIZE >
    using Queue = etl::queue< ITEM_TYPE, MAX_SIZE >;

    template< typename ITEM_TYPE >
    using IQueue = etl::iqueue< ITEM_TYPE >;
}    // namespace AsnPlus

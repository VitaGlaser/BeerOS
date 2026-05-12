#pragma once

#include "etl/stack.h"

namespace AsnPlus
{
    template< typename ITEM_TYPE, const size_t MAX_SIZE >
    using Stack = etl::stack< ITEM_TYPE, MAX_SIZE >;

    template< typename ITEM_TYPE >
    using IStack = etl::istack< ITEM_TYPE >;
}    // namespace AsnPlus

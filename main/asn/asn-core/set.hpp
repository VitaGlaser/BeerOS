#pragma once

#include "etl/set.h"

namespace AsnPlus
{
    template< typename ITEM_TYPE, const size_t MAX_SIZE >
    using Set = etl::set< ITEM_TYPE, MAX_SIZE >;

    template< typename ITEM_TYPE >
    using ISet = etl::iset< ITEM_TYPE >;
}    // namespace AsnPlus

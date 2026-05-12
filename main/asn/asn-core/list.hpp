#pragma once

#include "etl/list.h"

namespace AsnPlus
{
    template< typename ITEM_TYPE, const size_t MAX_SIZE >
    using List = etl::list< ITEM_TYPE, MAX_SIZE >;

    template< typename ITEM_TYPE >
    using IList = etl::ilist< ITEM_TYPE >;
}    // namespace AsnPlus

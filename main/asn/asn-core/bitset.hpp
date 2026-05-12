#pragma once

#include "etl/bitset.h"

namespace AsnPlus
{
    template< size_t ACTIVE_BITS, typename ELEMENT >
    using Bitset = etl::bitset< ACTIVE_BITS, ELEMENT >;

    template< size_t ACTIVE_BITS, typename ELEMENT >
    using BitsetExt = etl::bitset_ext< ACTIVE_BITS, ELEMENT >;
}    // namespace AsnPlus

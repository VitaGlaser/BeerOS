#pragma once

#include "etl/circular_buffer.h"

namespace AsnPlus
{
    template< typename ITEM_TYPE, const size_t MAX_SIZE >
    using RingBuffer = etl::circular_buffer< ITEM_TYPE, MAX_SIZE >;

    template< typename ITEM_TYPE >
    using IRingBuffer = etl::icircular_buffer< ITEM_TYPE >;

    template< typename ITEM_TYPE >
    using RingBufferExt = etl::circular_buffer_ext< ITEM_TYPE >;
}    // namespace AsnPlus

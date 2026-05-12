#pragma once

#include "etl/deque.h"

namespace AsnPlus
{
    template< typename T, const size_t MAX_SIZE >
    using Deque = etl::deque< T, MAX_SIZE >;

    template< typename T >
    using IDeque = etl::ideque< T >;
}    // namespace AsnPlus

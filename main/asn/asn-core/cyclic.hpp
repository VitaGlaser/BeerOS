#pragma once

#include "etl/cyclic_value.h"

namespace AsnPlus
{
    template< typename T, T FIRST, T LAST >
    using Cyclic = etl::cyclic_value< T, FIRST, LAST >;
}

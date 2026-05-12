#pragma once

#include "etl/flags.h"

namespace AsnPlus
{
    template< typename T, T MASK = etl::integral_limits< T >::max >
    using Flags = etl::flags< T, MASK >;
}

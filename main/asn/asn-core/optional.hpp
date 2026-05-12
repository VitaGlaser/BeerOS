#pragma once

#include "etl/optional.h"

namespace AsnPlus
{
    template< typename T >
    using Optional = etl::optional< T >;
}

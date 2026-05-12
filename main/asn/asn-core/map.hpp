#pragma once

#include "etl/map.h"

namespace AsnPlus
{
    template< typename KEY, typename VALUE, const size_t MAX_SIZE >
    using Map = etl::map< KEY, VALUE, MAX_SIZE >;

    template< typename KEY, typename VALUE >
    using IMap = etl::imap< KEY, VALUE >;
}    // namespace AsnPlus

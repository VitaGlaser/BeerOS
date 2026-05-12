#pragma once

#include "etl/delegate.h"

namespace AsnPlus
{
    template< typename Signature >
    using Delegate = etl::delegate< Signature >;
}

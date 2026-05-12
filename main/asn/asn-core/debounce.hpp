#pragma once

#include "etl/debounce.h"

namespace AsnPlus
{
    template< const uint16_t VALID_COUNT = 0, const uint16_t HOLD_COUNT = 0, const uint16_t REPEAT_COUNT = 0 >
    using Debounce = etl::debounce< VALID_COUNT, HOLD_COUNT, REPEAT_COUNT >;
}

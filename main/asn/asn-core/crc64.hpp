#pragma once

#include "etl/crc64_ecma.h"

namespace AsnPlus
{
    namespace Crc64
    {
        /*
            Polynomial: 0x42F0E1EBA9EA3693
        */
        template< size_t TABLE_SIZE >
        using Ecma = etl::crc64_ecma_t< TABLE_SIZE >;

    }    // namespace Crc64
}    // namespace AsnPlus

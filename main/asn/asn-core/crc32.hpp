#pragma once

#include "etl/crc32.h"
#include "etl/crc32_bzip2.h"
#include "etl/crc32_c.h"
#include "etl/crc32_d.h"
#include "etl/crc32_jamcrc.h"
#include "etl/crc32_mpeg2.h"
#include "etl/crc32_posix.h"
#include "etl/crc32_q.h"
#include "etl/crc32_xfer.h"

namespace AsnPlus
{
    namespace Crc32
    {
        /*
            Polynomial: 0x04C11DB7
            Initial value: 0xffffffff
            Output xor: 0xffffffff
            Reflected
        */
        template< size_t TABLE_SIZE >
        using Iso3309 = etl::crc32_t< TABLE_SIZE >;

        /*
            Polynomial: 0x04C11DB7
            Initial value: 0xffffffff
            Output xor: 0xffffffff
        */
        template< size_t TABLE_SIZE >
        using BZip2 = etl::crc32_bzip2_t< TABLE_SIZE >;

        /*
            Polynomial: 0x1EDC6F41
            Initial value: 0xffffffff
            Output xor: 0xffffffff
            Reflected
        */
        template< size_t TABLE_SIZE >
        using C = etl::crc32_c_t< TABLE_SIZE >;

        /*
            Polynomial: 0xA833982B
            Initial value: 0xffffffff
            Output xor: 0xffffffff
            Reflected
        */
        template< size_t TABLE_SIZE >
        using D = etl::crc32_d_t< TABLE_SIZE >;

        /*
            Polynomial: 0x04C11DB7
            Initial value: 0xffffffff
            Reflected
        */
        template< size_t TABLE_SIZE >
        using Jamrc = etl::crc32_jamrc_t< TABLE_SIZE >;

        /*
            Polynomial: 0x04C11DB7
            Initial value: 0xffffffff
        */
        template< size_t TABLE_SIZE >
        using Mpeg2 = etl::crc32_mpeg2_t< TABLE_SIZE >;

        /*
            Polynomial: 0x04C11DB7
            Initial value: 0
            Output xor: 0xffffffff
        */
        template< size_t TABLE_SIZE >
        using Posix = etl::crc32_posix_t< TABLE_SIZE >;

        /*
            Polynomial: 0x814141AB
            Initial value: 0
        */
        template< size_t TABLE_SIZE >
        using Q = etl::crc32_q_t< TABLE_SIZE >;

        /*
            Polynomial: 0x000000AF
            Initial value: 0
        */
        template< size_t TABLE_SIZE >
        using Xfer = etl::crc32_xfer_t< TABLE_SIZE >;
    }    // namespace Crc32
}    // namespace AsnPlus

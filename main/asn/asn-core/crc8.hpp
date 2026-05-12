#pragma once

#include "etl/crc8_ccitt.h"
#include "etl/crc8_cdma2000.h"
#include "etl/crc8_darc.h"
#include "etl/crc8_dvbs2.h"
#include "etl/crc8_ebu.h"
#include "etl/crc8_icode.h"
#include "etl/crc8_itu.h"
#include "etl/crc8_j1850.h"
#include "etl/crc8_j1850_zero.h"
#include "etl/crc8_maxim.h"
#include "etl/crc8_rohc.h"
#include "etl/crc8_wcdma.h"

namespace AsnPlus
{
    namespace Crc8
    {
        /*
            Polynomial: 0x07
            Initial value: 0
        */
        template< size_t TABLE_SIZE >
        using Ccitt = etl::crc8_ccitt_t< TABLE_SIZE >;

        /*
            Polynomial: 0x9B
            Initial value: 0xFF
        */
        template< size_t TABLE_SIZE >
        using Cdma2000 = etl::crc8_cdma2000_t< TABLE_SIZE >;

        /*
            Polynomial: 0x39
            Initial value: 0
            Reflected
        */
        template< size_t TABLE_SIZE >
        using Darc = etl::crc8_darc_t< TABLE_SIZE >;

        /*
            Polynomial: 0xD5
            Initial value: 0
        */
        template< size_t TABLE_SIZE >
        using Dvbs2 = etl::crc8_dvbs2_t< TABLE_SIZE >;

        /*
            Polynomial: 0x1D
            Initial value: 0xFF
            Reflected
        */
        template< size_t TABLE_SIZE >
        using Ebu = etl::crc8_ebu_t< TABLE_SIZE >;

        /*
            Polynomial: 0x1D
            Initial value: 0xFD
        */
        template< size_t TABLE_SIZE >
        using Icode = etl::crc8_icode_t< TABLE_SIZE >;

        /*
            Polynomial: 0x07
            Initial value: 0
            Output xor: 0x55
        */
        template< size_t TABLE_SIZE >
        using Itu = etl::crc8_itu_t< TABLE_SIZE >;

        /*
            Polynomial: 0x1D
            Initial value: 0xFF
            Output xor: 0xFF
        */
        template< size_t TABLE_SIZE >
        using J1850 = etl::crc8_j1850_t< TABLE_SIZE >;

        /*
            Polynomial: 0x1D
            Initial value: 0
        */
        template< size_t TABLE_SIZE >
        using J1850Zero = etl::crc8_j1850_zero_t< TABLE_SIZE >;

        /*
            Polynomial: 0x31
            Initial value: 0
            Reflected
        */
        template< size_t TABLE_SIZE >
        using Maxim = etl::crc8_maxim_t< TABLE_SIZE >;

        /*
            Polynomial: 0x07
            Initial value: 0xFF
            Reflected
        */
        template< size_t TABLE_SIZE >
        using Rohc = etl::crc8_rohc_t< TABLE_SIZE >;

        /*
            Polynomial: 0x9B
            Initial value: 0
            Reflected
        */
        template< size_t TABLE_SIZE >
        using Wcdma = etl::crc8_wcdma_t< TABLE_SIZE >;
    }    // namespace Crc8
}    // namespace AsnPlus

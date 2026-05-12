#pragma once

#include "etl/crc16.h"
#include "etl/crc16_a.h"
#include "etl/crc16_arc.h"
#include "etl/crc16_aug_ccitt.h"
#include "etl/crc16_buypass.h"
#include "etl/crc16_ccitt.h"
#include "etl/crc16_cdma2000.h"
#include "etl/crc16_dds110.h"
#include "etl/crc16_dectr.h"
#include "etl/crc16_dectx.h"
#include "etl/crc16_dnp.h"
#include "etl/crc16_en13757.h"
#include "etl/crc16_genibus.h"
#include "etl/crc16_kermit.h"
#include "etl/crc16_m17.h"
#include "etl/crc16_maxim.h"
#include "etl/crc16_mcrf4xx.h"
#include "etl/crc16_modbus.h"
#include "etl/crc16_profibus.h"
#include "etl/crc16_riello.h"
#include "etl/crc16_t10dif.h"
#include "etl/crc16_teledisk.h"
#include "etl/crc16_tms37157.h"
#include "etl/crc16_usb.h"
#include "etl/crc16_x25.h"
#include "etl/crc16_xmodem.h"

namespace AsnPlus
{
    namespace Crc16
    {
        /*
            Polynomial: 0x1021
            Initial value: 0xc6c6
            Reflected
        */
        template< size_t TABLE_SIZE >
        using A = etl::crc16_a_t< TABLE_SIZE >;

        /*
            Polynomial: 0x8005
            Reflected
        */
        template< size_t TABLE_SIZE >
        using Arc = etl::crc16_arc_t< TABLE_SIZE >;

        /*
            Polynomial: 0x8005
            Reflected
        */
        template< size_t TABLE_SIZE >
        using Ansi = etl::crc16_t< TABLE_SIZE >;

        /*
            Polynomial: 0x8005
        */
        template< size_t TABLE_SIZE >
        using Buypass = etl::crc16_buypass_t< TABLE_SIZE >;

        /*
            Polynomial: 0x1021
            Initial value: 0xffff
        */
        template< size_t TABLE_SIZE >
        using Ccitt = etl::crc16_ccitt_t< TABLE_SIZE >;

        /*
            Polynomial: 0x1021
            Initial value: 0x1d0f
        */
        template< size_t TABLE_SIZE >
        using CcittAug = etl::crc16_aug_ccitt_t< TABLE_SIZE >;

        /*
            Polynomial: 0xc867
            Initial value: 0xffff
        */
        template< size_t TABLE_SIZE >
        using Cdma2000 = etl::crc16_cdma2000_t< TABLE_SIZE >;

        /*
            Polynomial: 0x8005
            Initial value: 0x800d
        */
        template< size_t TABLE_SIZE >
        using Dds110 = etl::crc16_dds110_t< TABLE_SIZE >;

        /*
            Polynomial: 0x0589
            Output xor: 0x0001
        */
        template< size_t TABLE_SIZE >
        using DectR = etl::crc16_dect_r_t< TABLE_SIZE >;

        /*
            Polynomial: 0x0589
        */
        template< size_t TABLE_SIZE >
        using DectX = etl::crc16_dect_x_t< TABLE_SIZE >;

        /*
            Polynomial: 0x3D65
            Output xor: 0xFFFF
            Reflected
        */
        template< size_t TABLE_SIZE >
        using Dnp = etl::crc16_dnp_t< TABLE_SIZE >;

        /*
            Polynomial: 0x3D65
            Output xor: 0xFFFF
        */
        template< size_t TABLE_SIZE >
        using En13757 = etl::crc16_en13757_t< TABLE_SIZE >;

        /*
            Polynomial: 0x1021
            Initial value: 0xffff
            Output xor: 0xFFFF
        */
        template< size_t TABLE_SIZE >
        using Genibus = etl::crc16_genibus_t< TABLE_SIZE >;

        /*
            Polynomial: 0x1021
            Reflected
        */
        template< size_t TABLE_SIZE >
        using Kermit = etl::crc16_kermit_t< TABLE_SIZE >;

        /*
            Polynomial: 0x5935
            Initial value: 0xffff
        */
        template< size_t TABLE_SIZE >
        using M17 = etl::crc16_m17_t< TABLE_SIZE >;

        /*
            Polynomial: 0x8005
            Output xor: 0xFFFF
            Reflected
        */
        template< size_t TABLE_SIZE >
        using Maxim = etl::crc16_maxim_t< TABLE_SIZE >;

        /*
            Polynomial: 0x1021
            Initial value: 0xffff
            Reflected
        */
        template< size_t TABLE_SIZE >
        using Mcrf4xx = etl::crc16_mcrf4xx_t< TABLE_SIZE >;

        /*
            Polynomial: 0x8005
            Initial value: 0xffff
            Reflected
        */
        template< size_t TABLE_SIZE >
        using Modbus = etl::crc16_modbus_t< TABLE_SIZE >;

        /*
            Polynomial: 0x1DCF
            Initial value: 0xffff
            Output xor: 0xFFFF
        */
        template< size_t TABLE_SIZE >
        using Profibus = etl::crc16_profibus_t< TABLE_SIZE >;

        /*
            Polynomial: 0x1021
            Initial value: 0xb2aa
            Reflected
        */
        template< size_t TABLE_SIZE >
        using Riello = etl::crc16_riello_t< TABLE_SIZE >;

        /*
            Polynomial: 0x8BB7
        */
        template< size_t TABLE_SIZE >
        using T10dif = etl::crc16_t10dif_t< TABLE_SIZE >;

        /*
            Polynomial: 0xA097
        */
        template< size_t TABLE_SIZE >
        using Teledisk = etl::crc16_teledisk_t< TABLE_SIZE >;

        /*
            Polynomial: 0x1021
            Initial value: 0x89ec
            Reflected
        */
        template< size_t TABLE_SIZE >
        using Tms37157 = etl::crc16_tms37157_t< TABLE_SIZE >;

        /*
            Polynomial: 0x8005
            Initial value: 0xffff
            Output xor: 0xFFFF
            Reflected
        */
        template< size_t TABLE_SIZE >
        using Usb = etl::crc16_usb_t< TABLE_SIZE >;

        /*
            Polynomial: 0x1021
            Initial value: 0xffff
            Output xor: 0xFFFF
            Reflected
        */
        template< size_t TABLE_SIZE >
        using X25 = etl::crc16_x25_t< TABLE_SIZE >;

        /*
            Polynomial: 0x1021
        */
        template< size_t TABLE_SIZE >
        using Xmodem = etl::crc16_xmodem_t< TABLE_SIZE >;
    }    // namespace Crc16
}    // namespace AsnPlus

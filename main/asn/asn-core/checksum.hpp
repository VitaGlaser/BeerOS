#pragma once

#include "etl/checksum.h"

namespace AsnPlus
{
    namespace Checksum
    {
        template< typename POLICY >
        using Generic = etl::frame_check_sequence< POLICY >;

        template< typename T >
        using Sum = etl::checksum< T >;

        template< typename T >
        using Bsd = etl::bsd_checksum< T >;

        template< typename T >
        using Xor = etl::xor_checksum< T >;

        template< typename T >
        using XorRotate = etl::xor_rotate_checksum< T >;

        template< typename T >
        using Parity = etl::parity_checksum< T >;
    }    // namespace Checksum
}    // namespace AsnPlus

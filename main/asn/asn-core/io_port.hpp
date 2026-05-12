#pragma once

#include "etl/io_port.h"

namespace AsnPlus
{
    template< typename T, uintptr_t ADDRESS = 0 >
    using PortRW = io_port_rw< T, ADDRESS >;

    template< typename T, uintptr_t ADDRESS = 0 >
    using PortRO = io_port_ro< T, ADDRESS >;

    template< typename T, uintptr_t ADDRESS = 0 >
    using PortWO = io_port_wo< T, ADDRESS >;

    template< typename T, uintptr_t ADDRESS = 0 >
    using PortWOS = io_port_wos< T, ADDRESS >;
}    // namespace AsnPlus

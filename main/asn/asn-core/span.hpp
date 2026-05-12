#pragma once

#include "etl/span.h"

namespace AsnPlus
{
    template< typename T, size_t Extent = etl::dynamic_extent >
    using Span = etl::span< T, Extent >;

    template< size_t Extent = etl::dynamic_extent >
    using Bytes  = etl::span< uint8_t, Extent >;
    using ConstBytes = etl::span< const uint8_t, etl::dynamic_extent >;

    using IBytes = etl::span< uint8_t >;
    using IConstBytes = etl::span< const uint8_t >;
}    // namespace AsnPlus

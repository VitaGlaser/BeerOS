#pragma once

#include "etl/buffer_descriptors.h"

namespace AsnPlus
{
    template< typename BUFFER, size_t SIZE, size_t COUNT, typename FLAG = bool >
    using BufferDescriptors = etl::buffer_descriptors< BUFFER, SIZE, COUNT, FLAG >;
}

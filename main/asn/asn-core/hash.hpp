#pragma once

#include "etl/fnv_1.h"
#include "etl/hash.h"
#include "etl/jenkins.h"
#include "etl/murmur3.h"
#include "etl/pearson.h"

namespace AsnPlus
{
    namespace Hash
    {
        template< typename T >
        using Default = etl::hash< T >;

        template< size_t HASH_LENGTH >
        using Pearson = etl::pearson< HASH_LENGTH >;

        template< typename DATA_TYPE >
        using Murmur3 = etl::murmur3< DATA_TYPE >;

        using Jenkins = etl::jenkins;
        using Fnv1    = etl::fnv_1_32;
        using Fnv1a   = etl::fnv_1a_32;
    }    // namespace Hash
}    // namespace AsnPlus

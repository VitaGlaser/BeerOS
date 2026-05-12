#pragma once

#include "types.hpp"

namespace AsnPlus
{
    template< typename ITEM, size_t CAPACITY, size_t ID = 0 >
    struct InstanceStorage
    {
        using CONTAINER = Vector< ITEM *, CAPACITY >;

        InstanceStorage() { all_instances().push_back( (ITEM *) this ); }

        static CONTAINER & all_instances()
        {
            static CONTAINER container {};
            return container;
        }
    };
}    // namespace AsnPlus

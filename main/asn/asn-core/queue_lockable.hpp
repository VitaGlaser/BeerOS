#pragma once

#include "etl/queue_lockable.h"
#include "memory_model.hpp"

namespace AsnPlus
{
    template< typename T, size_t MAX_SIZE, MemoryModel MEMORY_MODEL = MemoryModel::U32 >
    using QueueLockable = etl::queue_lockable< T, MAX_SIZE, (size_t) MEMORY_MODEL >;

    template< typename T, MemoryModel MEMORY_MODEL = MemoryModel::U32 >
    using IQueueLockable = etl::iqueue_lockable< T, (size_t) MEMORY_MODEL >;
}    // namespace AsnPlus

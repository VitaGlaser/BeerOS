#pragma once

#include "etl/mutex.h"

namespace AsnPlus
{
    using Mutex = etl::mutex;

    template< typename MUTEX >
    using TLockGuard = etl::lock_guard< MUTEX >;
    using LockGuard  = TLockGuard< Mutex >;
}    // namespace AsnPlus

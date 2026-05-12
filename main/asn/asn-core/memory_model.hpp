#pragma once

#include "etl/memory_model.h"

namespace AsnPlus
{
    enum class MemoryModel
    {
        U8  = etl::memory_model::MEMORY_MODEL_SMALL,
        U16 = etl::memory_model::MEMORY_MODEL_MEDIUM,
        U32 = etl::memory_model::MEMORY_MODEL_LARGE,
        U64 = etl::memory_model::MEMORY_MODEL_HUGE,
    };
}    // namespace AsnPlus

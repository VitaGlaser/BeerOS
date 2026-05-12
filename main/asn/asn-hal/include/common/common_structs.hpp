#pragma once

#include "asn/asn-core/types.hpp"

namespace AsnPlus
{
    enum class Status : uint8_t
    {
        UNKNOWN = 0,
        OK,
        WARNING,
        ERROR
    };

    struct Config
    {
        uint64_t timestamp = 0;
    };

    struct Runtime
    {
        uint64_t timestamp = 0;
        Status   status    = Status::UNKNOWN;
    };

}    // namespace AsnPlus

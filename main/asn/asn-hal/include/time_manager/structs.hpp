#pragma once

#include "asn/asn-core/types.hpp"

#include "common/common_structs.hpp"

namespace AsnPlus
{
    struct TimeConfig: AsnPlus::Config
    {
        static constexpr uint8_t TIMEZONE_LENGTH = 64;
        static constexpr uint8_t TIMEZONE_NAME_LENGTH = 64;

        char timezone[ TIMEZONE_LENGTH ] {};
        char timezoneName[ TIMEZONE_NAME_LENGTH ] {};
    };

    struct TimeRuntime
    {
        uint64_t utcEpochMs = 0;
    };

    struct TimeChangeRequest
    {
        uint64_t utcEpochMs = 0;
    };

}    // namespace AsnPlus

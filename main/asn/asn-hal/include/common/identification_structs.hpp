#pragma once

#include "asn/asn-core/types.hpp"

namespace AsnPlus
{
    struct FirmwareInfo
    {
        uint32_t version;
        uint32_t dataModelVersion;
    };

    struct ManufactureInfo
    {
        static constexpr uint8_t UID_LENGTH = 16;

        enum class Environment : uint8_t
        {
            UNKNOWN,
            DEVELOP,
            STAGING,
            PRODUCTION
        };

        uint64_t cts               = 0;
        char     uid[ UID_LENGTH ] = { 'A', 'S', 'N', 'P', 'D', 'T', 'T', 'Y', 'P', 'R', '0', '0', '0', '0', '0', '0' };
        Environment environment    = Environment::UNKNOWN;
    };

    struct OwnerInfo
    {
        static constexpr uint8_t OWNER_ID_LENGTH = 32;
        static constexpr uint8_t NAME_LENGTH     = 32;

        uint64_t timestamp                       = 0;
        char     ownerId[ OWNER_ID_LENGTH ]      = {};
        char     deviceName[ NAME_LENGTH ]       = {};
    };
}    // namespace AsnPlus

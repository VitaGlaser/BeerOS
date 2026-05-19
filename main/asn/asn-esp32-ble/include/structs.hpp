#pragma once

#include "asn/asn-core/types.hpp"

namespace AsnPlus::Bluetooth
{
    enum class Status : uint8_t
    {
        DISCONNECTED,
        INIT,
        START,
        ADVERTISE,
        CONNECTED,
        UNKNOWN
    };

    struct Config
    {
        uint32_t timestamp;
        // 0 - Use defaults, otherwise set custom advertising intervals in units of 0.625 ms
        uint32_t minAdvertisingInterval = 0;
        // 0 - Use defaults, otherwise set custom advertising intervals in units of 0.625 ms
        uint32_t maxAdvertisingInterval = 0;
    };

    struct History
    {
        uint32_t timestamp;
        uint32_t sequenceNumber;
        Status   status;
    };

    struct State
    {
        uint32_t timestamp;
        Status   status;
        uint16_t connectionHandle;
    };
}    // namespace AsnPlus::Bluetooth

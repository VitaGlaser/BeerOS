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
    };

    struct History
    {
        uint32_t timestamp;
        uint32_t sequenceNumber;
        Status status;
    };

    struct State
    {
        uint32_t timestamp;
        Status status;
        uint16_t connectionHandle;
    };
}

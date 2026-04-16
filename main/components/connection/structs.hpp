#pragma once

#include "asn/asn-core/types.hpp"

namespace AsnPlus::Connection
{
    enum class State: uint8_t
    {
        UNKNOWN, 
        DISCONNECTED,
        CONNECTING,
        CONNECTED
    };

    struct Config
    {
        uint64_t timestamp    = 0;
        bool btEnabled = true;
        bool ethEnabled = false;
        bool wifiEnabled = false;
        bool lteEnabled = false;
        bool nbIotEnabled = false;
    };

    struct Runtime
    {
        State ethValue = State::DISCONNECTED;
        State btValue = State::DISCONNECTED;
        uint16_t btRssi = 0;
        State wifiValue = State::DISCONNECTED;
        uint16_t wifiRssi = 0;
        State lteValue = State::DISCONNECTED;
        uint16_t lteRssi = 0;
        State nbIotValue = State::DISCONNECTED;
        uint16_t nbIotRssi = 0;
    };
}
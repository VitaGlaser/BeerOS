#pragma once

#include "asn/asn-core/types.hpp"

namespace AsnPlus::Connection
{
    enum class Value: uint8_t
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
        Value btValue = Value::DISCONNECTED;
        uint16_t btRssi = 0;
        Value ethValue = Value::DISCONNECTED;
        uint16_t ethRssi = 0;
        Value wifiValue = Value::DISCONNECTED;
        uint16_t wifiRssi = 0;
        Value lteValue = Value::DISCONNECTED;
        uint16_t lteRssi = 0;
        Value nbIotValue = Value::DISCONNECTED;
        uint16_t nbIotRssi = 0;
    };
}
#pragma once

#include "asn/asn-core/types.hpp"

namespace AsnPlus::Cloud
{
    static constexpr const char DEVELOP_URL_BASE[]           = "http://192.168.4.72";
    static constexpr const char STAGING_URL_BASE[]           = "";
    static constexpr const char PRODUCTION_URL_BASE[]        = "";

    static constexpr const char MOCK_TIME_CONFIG_URL[]       = "timeConfig";
    static constexpr const char MOCK_DEVICE_CONFIG_URL[]     = "deviceConfig";
    static constexpr const char MOCK_CONNECTION_CONFIG_URL[] = "networkConfig";
    static constexpr const char MOCK_CHANNEL_CONFIG_URL[]    = "channelConfig";
    static constexpr const char MOCK_STATE_URL[]             = "state";
    static constexpr const char MOCK_CHANNEL_HISTORY_URL[]   = "channelHistory";

}    // namespace AsnPlus::Cloud

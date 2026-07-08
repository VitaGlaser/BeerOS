#pragma once

#include "asn/asn-core/types.hpp"

namespace AsnPlus::Cloud
{
    static constexpr const char DEVELOP_URL_BASE[] = "https://europe-west3-cleverpub-dev.cloudfunctions.net/deviceApi";
    static constexpr const char STAGING_URL_BASE[] = "https://europe-west3-cleverpub-dev.cloudfunctions.net/deviceApi";
    //static constexpr const char DEVELOP_URL_BASE[] = "https://europe-west3-cleverpub-prod.cloudfunctions.net/deviceApi";
  //  static constexpr const char STAGING_URL_BASE[] = "https://europe-west3-cleverpub-prod.cloudfunctions.net/deviceApi";
    static constexpr const char PRODUCTION_URL_BASE[]        = "";

    static constexpr const char MOCK_TIME_CONFIG_URL[]       = "timeConfig";
    static constexpr const char MOCK_DEVICE_CONFIG_URL[]     = "deviceConfig";
    static constexpr const char MOCK_CONNECTION_CONFIG_URL[] = "networkConfig";
    static constexpr const char MOCK_MQTT_CONFIG_URL[]       = "mqttConfig";
    static constexpr const char MOCK_CHANNEL_CONFIG_URL[]    = "channelConfig";
    static constexpr const char MOCK_STATE_URL[]             = "state";
    static constexpr const char MOCK_CHANNEL_HISTORY_URL[]   = "channelHistory";
    static constexpr const char PROFILE_WEBHOOK_URL[]        = "https://automation.teximp.cz/beeros";
    static constexpr const char UNIT_STATUS_URL[]            = "https://beeros.revosoft.cz/units/api/heartbeat";

}    // namespace AsnPlus::Cloud

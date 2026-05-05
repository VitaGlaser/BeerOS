#pragma once

#include "cJSON.h"

#include "asn/asn-core/types.hpp"

namespace AsnPlus::Cloud
{
    namespace StateResponseJson
    {
        static constexpr const char TIMESTAMP_TAG[]                   = "timestamp";
        static constexpr const char TIME_CONFIG_TIMESTAMP_TAG[]       = "timeConfigTimestamp";
        static constexpr const char DEVICE_CONFIG_TIMESTAMP_TAG[]     = "deviceConfigTimestamp";
        static constexpr const char NETWORK_CONFIG_TIMESTAMP_TAG[]    = "networkConfigTimestamp";
        static constexpr const char MQTT_CONFIG_TIMESTAMP_TAG[]       = "mqttConfigTimestamp";
        static constexpr const char CHANNEL_CONFIG_TIMESTAMPS_TAG[]   = "channelConfigTimestamps";
    }    // namespace StateResponseJson

    /**
     * @brief Server reply to a state POST.
     *        Contains authoritative config timestamps the device uses to decide
     *        which configs need to be re-fetched.
     */
    struct StateResponse
    {
        static constexpr uint8_t CHANNEL_COUNT = 4;

        uint64_t timestamp                                = 0;
        uint64_t timeConfigTimestamp                      = 0;
        uint64_t deviceConfigTimestamp                    = 0;
        uint64_t networkConfigTimestamp                   = 0;
        uint64_t mqttConfigTimestamp                      = 0;
        uint64_t channelConfigTimestamps[ CHANNEL_COUNT ] = { 0, 0, 0, 0 };
    };

    void fromJson( StateResponse & response, cJSON * json )
    {
        response.timestamp = static_cast< uint64_t >(
            cJSON_GetNumberValue( cJSON_GetObjectItem( json, StateResponseJson::TIMESTAMP_TAG ) )
        );
        response.timeConfigTimestamp = static_cast< uint64_t >(
            cJSON_GetNumberValue( cJSON_GetObjectItem( json, StateResponseJson::TIME_CONFIG_TIMESTAMP_TAG ) )
        );
        response.deviceConfigTimestamp = static_cast< uint64_t >(
            cJSON_GetNumberValue( cJSON_GetObjectItem( json, StateResponseJson::DEVICE_CONFIG_TIMESTAMP_TAG ) )
        );
        response.networkConfigTimestamp = static_cast< uint64_t >(
            cJSON_GetNumberValue( cJSON_GetObjectItem( json, StateResponseJson::NETWORK_CONFIG_TIMESTAMP_TAG ) )
        );
        response.mqttConfigTimestamp = static_cast< uint64_t >(
            cJSON_GetNumberValue( cJSON_GetObjectItem( json, StateResponseJson::MQTT_CONFIG_TIMESTAMP_TAG ) )
        );

        cJSON * channelTimestamps = cJSON_GetObjectItem( json, StateResponseJson::CHANNEL_CONFIG_TIMESTAMPS_TAG );
        if ( channelTimestamps && cJSON_IsArray( channelTimestamps ) )
        {
            uint8_t i    = 0;
            cJSON * item = nullptr;
            cJSON_ArrayForEach( item, channelTimestamps )
            {
                if ( i >= StateResponse::CHANNEL_COUNT ) break;
                if ( cJSON_IsNumber( item ) )
                    response.channelConfigTimestamps[ i ] = static_cast< uint64_t >( item->valuedouble );
                ++i;
            }
        }
    }

}    // namespace AsnPlus::Cloud

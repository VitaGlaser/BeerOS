#pragma once

#include "cJSON.h"

#include "asn/asn-core/types.hpp"

#include "components/measurement/event_monitor.hpp"

namespace AsnPlus::Cloud
{
    // MARK: JSON tags

    namespace ChannelEventJson
    {
        static constexpr const char CONFIG_TIMESTAMP_TAG[] = "configTimestamp";
        static constexpr const char SEQ_NUMBER_TAG[]       = "seqNumber";
        static constexpr const char START_TS_TAG[]         = "startTimestamp";
        static constexpr const char END_TS_TAG[]           = "endTimestamp";
        static constexpr const char SYNCED_TAG[]           = "synced";
        static constexpr const char EVENT_TYPE_TAG[]       = "eventType";
        static constexpr const char VOLUME_TAG[]           = "volume";
        static constexpr const char PULSE_COUNT_TAG[]      = "pulseCount";
        static constexpr const char CLASSIFICATION_TAG[]   = "classification";
        static constexpr const char AVG_TEMPERATURE_TAG[]  = "avgTemperature";
        static constexpr const char AVG_CONDUCTIVITY_TAG[] = "avgConductivity";
        static constexpr const char FLOW_HISTORY_TAG[]     = "flowProfile";
        static constexpr const char VOLUME_HISTORY_TAG[]   = "volumeProfile";
    }    // namespace ChannelEventJson

    namespace ChannelEventRequestJson
    {
        static constexpr const char EVENT_TAG[] = "event";
    }    // namespace ChannelEventRequestJson

    // MARK: toJson

    void toJson( EventMonitor::Event & event, cJSON * json, bool includeProfiles = true )
    {
        cJSON_AddNumberToObject(
            json, ChannelEventJson::CONFIG_TIMESTAMP_TAG, static_cast< double >( event.configTimestamp )
        );
        cJSON_AddNumberToObject(
            json, ChannelEventJson::SEQ_NUMBER_TAG, static_cast< double >( event.sequenceNumber )
        );
        cJSON_AddBoolToObject( json, ChannelEventJson::SYNCED_TAG, event.synced );
        cJSON_AddNumberToObject(
            json, ChannelEventJson::EVENT_TYPE_TAG, static_cast< uint8_t >( event.type )
        );
        cJSON_AddNumberToObject( json, ChannelEventJson::START_TS_TAG, static_cast< double >( event.startTimestamp ) );
        cJSON_AddNumberToObject( json, ChannelEventJson::END_TS_TAG, static_cast< double >( event.endTimestamp ) );
        cJSON_AddNumberToObject( json, ChannelEventJson::VOLUME_TAG, static_cast< double >( event.volume ) );
        cJSON_AddNumberToObject( json, ChannelEventJson::PULSE_COUNT_TAG, static_cast< double >( event.pulseCount ) );
        cJSON_AddNumberToObject(
            json, ChannelEventJson::CLASSIFICATION_TAG, static_cast< double >( event.classification )
        );
        cJSON_AddNumberToObject(
            json, ChannelEventJson::AVG_TEMPERATURE_TAG, static_cast< double >( event.avgTemperature )
        );
        cJSON_AddNumberToObject(
            json, ChannelEventJson::AVG_CONDUCTIVITY_TAG, static_cast< double >( event.avgConductivity )
        );

        if ( includeProfiles )
        {
            cJSON * flowHistoryJson = cJSON_CreateArray();
            for ( uint16_t i = 0; i < EventMonitor::Event::HISTORY_SIZE; ++i )
            {
                cJSON_AddItemToArray(
                    flowHistoryJson, cJSON_CreateNumber( static_cast< double >( event.flowProfile[ i ] ) )
                );
            }
            cJSON_AddItemToObject( json, ChannelEventJson::FLOW_HISTORY_TAG, flowHistoryJson );

            cJSON * volumeHistoryJson = cJSON_CreateArray();
            for ( uint16_t i = 0; i < EventMonitor::Event::HISTORY_SIZE; ++i )
            {
                cJSON_AddItemToArray(
                    volumeHistoryJson, cJSON_CreateNumber( static_cast< double >( event.volumeProfile[ i ] ) )
                );
            }
            cJSON_AddItemToObject( json, ChannelEventJson::VOLUME_HISTORY_TAG, volumeHistoryJson );
        }
    }

    struct ChannelHistoryRequest
    {
        EventMonitor::Event & event;
    };

    void toJson( ChannelHistoryRequest & request, cJSON * json )
    {
        cJSON * eventJson = cJSON_CreateObject();
        toJson( request.event, eventJson );
        cJSON_AddItemToObject( json, ChannelEventRequestJson::EVENT_TAG, eventJson );
    }

}    // namespace AsnPlus::Cloud

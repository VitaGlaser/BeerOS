#pragma once

#include "cJSON.h"

#include "asn/asn-core/types.hpp"

#include "components/measurement/event_monitor.hpp"

namespace AsnPlus::Cloud
{
    // TODO (DK): Add the missing parametersm
    // MARK: JSON tags

    namespace ChannelEventJson
    {
        static constexpr const char SEQ_NUMBER_TAG[]    = "seqNumber";
        static constexpr const char SYNCED_TAG[]        = "synced";
        static constexpr const char START_TS_TAG[]      = "startTimestamp";
        static constexpr const char END_TS_TAG[]        = "endTimestamp";
        static constexpr const char FLOW_HISTORY_TAG[]  = "flowHistory";
        static constexpr const char TEMP_HISTORY_TAG[]  = "tempHistory";
        static constexpr const char PRESS_HISTORY_TAG[] = "pressureHistory";
    }    // namespace ChannelEventJson

    namespace ChannelEventRequestJson
    {
        static constexpr const char EVENT_TAG[] = "event";
    }    // namespace ChannelEventRequestJson

    // MARK: toJson

    void toJson( EventMonitor::Event & event, cJSON * json )
    {
        cJSON_AddNumberToObject( json, ChannelEventJson::SEQ_NUMBER_TAG, static_cast< double >( event.sequenceNumber ) );
        cJSON_AddBoolToObject( json, ChannelEventJson::SYNCED_TAG, event.synced );
        cJSON_AddNumberToObject(
            json, ChannelEventJson::START_TS_TAG, static_cast< double >( event.startTimestamp )
        );
        cJSON_AddNumberToObject( json, ChannelEventJson::END_TS_TAG, static_cast< double >( event.endTimestamp ) );

        cJSON * flowArr = cJSON_CreateArray();
        for ( uint8_t i = 0; i < EventMonitor::Event::HISTORY_SIZE; ++i )
            cJSON_AddItemToArray( flowArr, cJSON_CreateNumber( event.flowHistory[ i ] ) );
        cJSON_AddItemToObject( json, ChannelEventJson::FLOW_HISTORY_TAG, flowArr );

        cJSON * tempArr = cJSON_CreateArray();
        for ( uint8_t i = 0; i < EventMonitor::Event::HISTORY_SIZE; ++i )
            cJSON_AddItemToArray( tempArr, cJSON_CreateNumber( event.temperatureHistory[ i ] ) );
        cJSON_AddItemToObject( json, ChannelEventJson::TEMP_HISTORY_TAG, tempArr );

        cJSON * pressArr = cJSON_CreateArray();
        for ( uint8_t i = 0; i < EventMonitor::Event::HISTORY_SIZE; ++i )
            cJSON_AddItemToArray( pressArr, cJSON_CreateNumber( event.pressureHistory[ i ] ) );
        cJSON_AddItemToObject( json, ChannelEventJson::PRESS_HISTORY_TAG, pressArr );
    }

}    // namespace AsnPlus::Cloud

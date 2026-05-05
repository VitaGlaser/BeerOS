#pragma once

#include "cJSON.h"

#include "asn/asn-core/types.hpp"

#include "asn/asn-hal/include/time_manager/structs.hpp"

namespace AsnPlus::Cloud
{
    namespace TimeConfigJson
    {
        static constexpr const char TIMESTAMP_TAG[]     = "timestamp";
        static constexpr const char TIMEZONE_TAG[]      = "timezone";
        static constexpr const char TIMEZONE_NAME_TAG[] = "timezoneName";
    }    // namespace TimeConfigJson

    void toJson( AsnPlus::TimeConfig & config, cJSON * json )
    {
        cJSON_AddNumberToObject( json, TimeConfigJson::TIMESTAMP_TAG, static_cast< double >( config.timestamp ) );
        cJSON_AddStringToObject( json, TimeConfigJson::TIMEZONE_TAG, config.timezone );
        cJSON_AddStringToObject( json, TimeConfigJson::TIMEZONE_NAME_TAG, config.timezoneName );
    }

    void fromJson( AsnPlus::TimeConfig & config, cJSON * json )
    {
        config.timestamp = static_cast< uint64_t >(
            cJSON_GetNumberValue( cJSON_GetObjectItem( json, TimeConfigJson::TIMESTAMP_TAG ) )
        );

        cJSON * tz = cJSON_GetObjectItem( json, TimeConfigJson::TIMEZONE_TAG );
        if ( tz && cJSON_IsString( tz ) )
            strncpy( config.timezone, tz->valuestring, AsnPlus::TimeConfig::TIMEZONE_LENGTH - 1 );

        cJSON * tzName = cJSON_GetObjectItem( json, TimeConfigJson::TIMEZONE_NAME_TAG );
        if ( tzName && cJSON_IsString( tzName ) )
            strncpy( config.timezoneName, tzName->valuestring, AsnPlus::TimeConfig::TIMEZONE_NAME_LENGTH - 1 );
    }

}    // namespace AsnPlus::Cloud

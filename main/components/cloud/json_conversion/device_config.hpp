#pragma once

#include "cJSON.h"

#include "asn/asn-core/types.hpp"

#include "asn/asn-hal/common/identification_structs.hpp"

namespace AsnPlus::Cloud
{
    namespace DeviceConfigJson
    {
        static constexpr const char TIMESTAMP_TAG[] = "timestamp";
        static constexpr const char OWNER_ID_TAG[]  = "ownerId";
        static constexpr const char ORG_ID_TAG[]    = "orgId";
    }    // namespace DeviceConfigJson

    struct DeviceConfig
    {
        uint64_t timestamp                             = 0;
        char     ownerId[ OwnerInfo::OWNER_ID_LENGTH ] = { 0 };
        char     orgId[ OwnerInfo::OWNER_ID_LENGTH ]   = { 0 };
    };

    void toJson( DeviceConfig & config, cJSON * json )
    {
        cJSON_AddNumberToObject( json, DeviceConfigJson::TIMESTAMP_TAG, static_cast< double >( config.timestamp ) );
        cJSON_AddStringToObject( json, DeviceConfigJson::OWNER_ID_TAG, config.ownerId );
        cJSON_AddStringToObject( json, DeviceConfigJson::ORG_ID_TAG, config.orgId );
    }

    void fromJson( DeviceConfig & config, cJSON * json )
    {
        config.timestamp = static_cast< uint64_t >(
            cJSON_GetNumberValue( cJSON_GetObjectItem( json, DeviceConfigJson::TIMESTAMP_TAG ) )
        );

        cJSON * ownerId = cJSON_GetObjectItem( json, DeviceConfigJson::OWNER_ID_TAG );
        if ( ownerId && cJSON_IsString( ownerId ) )
            strncpy( config.ownerId, ownerId->valuestring, OwnerInfo::OWNER_ID_LENGTH - 1 );

        cJSON * orgId = cJSON_GetObjectItem( json, DeviceConfigJson::ORG_ID_TAG );
        if ( orgId && cJSON_IsString( orgId ) )
            strncpy( config.orgId, orgId->valuestring, OwnerInfo::OWNER_ID_LENGTH - 1 );
    }
}    // namespace AsnPlus::Cloud

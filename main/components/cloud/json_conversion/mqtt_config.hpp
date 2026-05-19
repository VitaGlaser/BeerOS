#pragma once

#include "cJSON.h"

#include "asn/asn-hal/include/mqtt/client.hpp"

namespace AsnPlus::Cloud::MqttConfigJson
{
    static constexpr const char TIMESTAMP_TAG[]      = "timestamp";
    static constexpr const char ENABLED_TAG[]        = "enabled";
    static constexpr const char BROKER_URI_TAG[]     = "brokerUri";
    static constexpr const char USERNAME_TAG[]       = "username";
    static constexpr const char PASSWORD_TAG[]       = "password";
    static constexpr const char KEEPALIVE_S_TAG[]    = "keepalive";
    static constexpr const char USE_TLS_TAG[]        = "useTls";
    static constexpr const char AUTO_RECONNECT_TAG[] = "autoReconnect";
}    // namespace AsnPlus::Cloud::MqttConfigJson

// MARK: toJson / fromJson — placed in AsnPlus::Mqtt so ADL finds them via Mqtt::IClient::Config

namespace AsnPlus::Mqtt
{
    void toJson( IClient::Config & config, cJSON * json )
    {
        using namespace Cloud::MqttConfigJson;
        cJSON_AddNumberToObject( json, TIMESTAMP_TAG, static_cast< double >( config.timestamp ) );
        cJSON_AddBoolToObject( json, ENABLED_TAG, config.enabled );
        cJSON_AddStringToObject( json, BROKER_URI_TAG, config.brokerUri );
        cJSON_AddStringToObject( json, USERNAME_TAG, config.username );
        cJSON_AddStringToObject( json, PASSWORD_TAG, config.password );
        cJSON_AddNumberToObject( json, KEEPALIVE_S_TAG, config.keepalive );
        cJSON_AddBoolToObject( json, USE_TLS_TAG, config.useTls );
        cJSON_AddBoolToObject( json, AUTO_RECONNECT_TAG, config.autoReconnect );
    }

    void fromJson( IClient::Config & config, cJSON * json )
    {
        using namespace Cloud::MqttConfigJson;
        config.timestamp =
            static_cast< uint64_t >( cJSON_GetNumberValue( cJSON_GetObjectItem( json, TIMESTAMP_TAG ) ) );

        config.enabled       = cJSON_IsTrue( cJSON_GetObjectItem( json, ENABLED_TAG ) );
        config.useTls        = cJSON_IsTrue( cJSON_GetObjectItem( json, USE_TLS_TAG ) );
        config.autoReconnect = cJSON_IsTrue( cJSON_GetObjectItem( json, AUTO_RECONNECT_TAG ) );

        cJSON * brokerUri    = cJSON_GetObjectItem( json, BROKER_URI_TAG );
        if ( brokerUri && cJSON_IsString( brokerUri ) )
            StringExt( config.brokerUri, config.brokerUri, IClient::Config::BROKER_URI_SIZE ).assign( brokerUri->valuestring );

        cJSON * username = cJSON_GetObjectItem( json, USERNAME_TAG );
        if ( username && cJSON_IsString( username ) )
            StringExt( config.username, config.username, IClient::Config::USERNAME_SIZE ).assign( username->valuestring );

        cJSON * password = cJSON_GetObjectItem( json, PASSWORD_TAG );
        if ( password && cJSON_IsString( password ) )
            StringExt( config.password, config.password, IClient::Config::PASSWORD_SIZE ).assign( password->valuestring );

        cJSON * keepalive = cJSON_GetObjectItem( json, KEEPALIVE_S_TAG );
        if ( keepalive && cJSON_IsNumber( keepalive ) )
            config.keepalive = static_cast< uint16_t >( keepalive->valuedouble );
    }
}    // namespace AsnPlus::Mqtt

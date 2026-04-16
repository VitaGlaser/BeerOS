#pragma once

#include "cJSON.h"

#include "asn/asn-core/types.hpp"
#include "asn/asn-core/vector.hpp"

#include "components/measurement/channel.hpp"

namespace AsnPlus::Cloud
{
    using ChannelConfig = Channel::Config;

    // MARK: JSON tags

    namespace SensorConfigJson
    {
        static constexpr const char TIMESTAMP_TAG[] = "timestamp";
        static constexpr const char ENABLED_TAG[]   = "enabled";
        static constexpr const char ID_TAG[]        = "id";
    }    // namespace SensorConfigJson

    namespace ClassificationConfigJson
    {
        static constexpr const char NAME_TAG[]       = "name";
        static constexpr const char MIN_VOLUME_TAG[] = "minVolume";
        static constexpr const char TYP_VOLUME_TAG[] = "typVolume";
        static constexpr const char MAX_VOLUME_TAG[] = "maxVolume";
    }    // namespace ClassificationConfigJson

    namespace ChannelConfigJson
    {
        static constexpr const char TIMESTAMP_TAG[]             = "timestamp";
        static constexpr const char ENABLED_TAG[]               = "enabled";
        static constexpr const char FLOW_TYPE_TAG[]             = "flowType";
        static constexpr const char FLOW_CONFIG_TAG[]           = "flowConfig";
        static constexpr const char TEMP_TYPE_TAG[]             = "temperatureType";
        static constexpr const char TEMP_CONFIG_TAG[]           = "temperatureConfig";
        static constexpr const char PRESS_TYPE_TAG[]             = "pressureType";
        static constexpr const char PRESS_CONFIG_TAG[]           = "pressureConfig";
        static constexpr const char COND_TYPE_TAG[]              = "conductivityType";
        static constexpr const char COND_CONFIG_TAG[]            = "conductivityConfig";
        static constexpr const char BEVERAGE_ID_TAG[]            = "beverageId";
        static constexpr const char CLASSIFICATION_CONFIG_TAG[] = "classificationConfigs:";
        static constexpr const char TAP_TIMEOUT_TAG[]           = "tapTimeoutMs";
        static constexpr const char TANK_CAPACITY_TAG[]         = "tankCapacity";
        static constexpr const char CLEANING_VOLUME_THR_TAG[]   = "cleaningVolumeThr";
    }    // namespace ChannelConfigJson

    // MARK: toJson / fromJson — SensorConfig

    void toJson( Sensor::Config & config, cJSON * json )
    {
        cJSON_AddNumberToObject( json, SensorConfigJson::TIMESTAMP_TAG, static_cast< double >( config.timestamp ) );
        cJSON_AddBoolToObject( json, SensorConfigJson::ENABLED_TAG, config.enabled );
        char id_str[ 17 ];
        snprintf( id_str, sizeof( id_str ), "%llx", static_cast< unsigned long long >( config.id ) );
        cJSON_AddStringToObject( json, SensorConfigJson::ID_TAG, id_str );
    }

    void fromJson( Sensor::Config & config, cJSON * json )
    {
        config.timestamp = static_cast< uint64_t >(
            cJSON_GetNumberValue( cJSON_GetObjectItem( json, SensorConfigJson::TIMESTAMP_TAG ) )
        );

        cJSON * enabled = cJSON_GetObjectItem( json, SensorConfigJson::ENABLED_TAG );
        if ( enabled && cJSON_IsBool( enabled ) ) config.enabled = cJSON_IsTrue( enabled );

        cJSON * id = cJSON_GetObjectItem( json, SensorConfigJson::ID_TAG );
        if ( id && cJSON_IsString( id ) )
            config.id = static_cast< uint64_t >( strtoull( id->valuestring, nullptr, 16 ) );
    }

    // MARK: toJson / fromJson — ClassificationConfig

    void toJson( Channel::Config::ClassificationConfig & config, cJSON * json )
    {
        cJSON_AddStringToObject( json, ClassificationConfigJson::NAME_TAG, config.name );
        cJSON_AddNumberToObject( json, ClassificationConfigJson::MIN_VOLUME_TAG, config.minVolume );
        cJSON_AddNumberToObject( json, ClassificationConfigJson::TYP_VOLUME_TAG, config.typVolume );
        cJSON_AddNumberToObject( json, ClassificationConfigJson::MAX_VOLUME_TAG, config.maxVolume );
    }

    void fromJson( Channel::Config::ClassificationConfig & config, cJSON * json )
    {
        cJSON * name = cJSON_GetObjectItem( json, ClassificationConfigJson::NAME_TAG );
        if ( name && cJSON_IsString( name ) )
            strncpy( config.name, name->valuestring, Channel::Config::ClassificationConfig::NAME_LENGTH - 1 );

        cJSON * minVol = cJSON_GetObjectItem( json, ClassificationConfigJson::MIN_VOLUME_TAG );
        if ( minVol && cJSON_IsNumber( minVol ) ) config.minVolume = static_cast< uint32_t >( minVol->valuedouble );

        cJSON * typVol = cJSON_GetObjectItem( json, ClassificationConfigJson::TYP_VOLUME_TAG );
        if ( typVol && cJSON_IsNumber( typVol ) ) config.typVolume = static_cast< uint32_t >( typVol->valuedouble );

        cJSON * maxVol = cJSON_GetObjectItem( json, ClassificationConfigJson::MAX_VOLUME_TAG );
        if ( maxVol && cJSON_IsNumber( maxVol ) ) config.maxVolume = static_cast< uint32_t >( maxVol->valuedouble );
    }

    // MARK: toJson / fromJson — ChannelConfig

    void toJson( ChannelConfig & config, cJSON * json )
    {
        cJSON_AddNumberToObject( json, ChannelConfigJson::TIMESTAMP_TAG, static_cast< double >( config.timestamp ) );
        cJSON_AddBoolToObject( json, ChannelConfigJson::ENABLED_TAG, config.enabled );

        cJSON_AddNumberToObject( json, ChannelConfigJson::FLOW_TYPE_TAG, static_cast< uint8_t >( config.flowType ) );
        cJSON * flowConfigJson = cJSON_CreateObject();
        toJson( config.flowConfig, flowConfigJson );
        cJSON_AddItemToObject( json, ChannelConfigJson::FLOW_CONFIG_TAG, flowConfigJson );

        cJSON_AddNumberToObject(
            json, ChannelConfigJson::TEMP_TYPE_TAG, static_cast< uint8_t >( config.temperatureType )
        );
        cJSON * tempConfigJson = cJSON_CreateObject();
        toJson( config.temperatureConfig, tempConfigJson );
        cJSON_AddItemToObject( json, ChannelConfigJson::TEMP_CONFIG_TAG, tempConfigJson );

        cJSON_AddNumberToObject(
            json, ChannelConfigJson::PRESS_TYPE_TAG, static_cast< uint8_t >( config.pressureType )
        );
        cJSON * pressConfigJson = cJSON_CreateObject();
        toJson( config.pressureConfig, pressConfigJson );
        cJSON_AddItemToObject( json, ChannelConfigJson::PRESS_CONFIG_TAG, pressConfigJson );

        cJSON_AddNumberToObject(
            json, ChannelConfigJson::COND_TYPE_TAG, static_cast< uint8_t >( config.conductivityType )
        );
        cJSON * condConfigJson = cJSON_CreateObject();
        toJson( config.conductivityConfig, condConfigJson );
        cJSON_AddItemToObject( json, ChannelConfigJson::COND_CONFIG_TAG, condConfigJson );

        cJSON_AddNumberToObject( json, ChannelConfigJson::BEVERAGE_ID_TAG, config.beverageId );

        cJSON * classificationConfigs = cJSON_CreateArray();
        for ( uint8_t i = 0; i < Channel::Config::CLASSIFICATION_CONFIG_COUNT; ++i )
        {
            cJSON * sizeJson = cJSON_CreateObject();
            toJson( config.classificationConfigs[ i ], sizeJson );
            cJSON_AddItemToArray( classificationConfigs, sizeJson );
        }
        cJSON_AddItemToObject( json, ChannelConfigJson::CLASSIFICATION_CONFIG_TAG, classificationConfigs );

        cJSON_AddNumberToObject( json, ChannelConfigJson::TAP_TIMEOUT_TAG, config.tapTimeoutMs );
        cJSON_AddNumberToObject( json, ChannelConfigJson::TANK_CAPACITY_TAG, config.tankCapacity );
        cJSON_AddNumberToObject( json, ChannelConfigJson::CLEANING_VOLUME_THR_TAG, config.cleaningVolumeThr );
    }

    void fromJson( ChannelConfig & config, cJSON * json )
    {
        config.timestamp = static_cast< uint64_t >(
            cJSON_GetNumberValue( cJSON_GetObjectItem( json, ChannelConfigJson::TIMESTAMP_TAG ) )
        );

        cJSON * enabled = cJSON_GetObjectItem( json, ChannelConfigJson::ENABLED_TAG );
        if ( enabled && cJSON_IsBool( enabled ) ) config.enabled = cJSON_IsTrue( enabled );

        cJSON * flowType = cJSON_GetObjectItem( json, ChannelConfigJson::FLOW_TYPE_TAG );
        if ( flowType && cJSON_IsNumber( flowType ) )
            config.flowType = static_cast< Channel::Config::FlowType >( flowType->valuedouble );

        cJSON * flowConfigJson = cJSON_GetObjectItem( json, ChannelConfigJson::FLOW_CONFIG_TAG );
        if ( flowConfigJson && cJSON_IsObject( flowConfigJson ) ) fromJson( config.flowConfig, flowConfigJson );

        cJSON * tempType = cJSON_GetObjectItem( json, ChannelConfigJson::TEMP_TYPE_TAG );
        if ( tempType && cJSON_IsNumber( tempType ) )
            config.temperatureType = static_cast< Channel::Config::TemperatureType >( tempType->valuedouble );

        cJSON * tempConfigJson = cJSON_GetObjectItem( json, ChannelConfigJson::TEMP_CONFIG_TAG );
        if ( tempConfigJson && cJSON_IsObject( tempConfigJson ) ) fromJson( config.temperatureConfig, tempConfigJson );

        cJSON * pressType = cJSON_GetObjectItem( json, ChannelConfigJson::PRESS_TYPE_TAG );
        if ( pressType && cJSON_IsNumber( pressType ) )
            config.pressureType = static_cast< Channel::Config::PressureType >( pressType->valuedouble );

        cJSON * pressConfigJson = cJSON_GetObjectItem( json, ChannelConfigJson::PRESS_CONFIG_TAG );
        if ( pressConfigJson && cJSON_IsObject( pressConfigJson ) ) fromJson( config.pressureConfig, pressConfigJson );

        cJSON * condType = cJSON_GetObjectItem( json, ChannelConfigJson::COND_TYPE_TAG );
        if ( condType && cJSON_IsNumber( condType ) )
            config.conductivityType = static_cast< Channel::Config::ConductivityType >( condType->valuedouble );

        cJSON * condConfigJson = cJSON_GetObjectItem( json, ChannelConfigJson::COND_CONFIG_TAG );
        if ( condConfigJson && cJSON_IsObject( condConfigJson ) ) fromJson( config.conductivityConfig, condConfigJson );

        cJSON * beverageId = cJSON_GetObjectItem( json, ChannelConfigJson::BEVERAGE_ID_TAG );
        if ( beverageId && cJSON_IsNumber( beverageId ) )
            config.beverageId = static_cast< uint16_t >( beverageId->valuedouble );

        cJSON * classificationConfigs = cJSON_GetObjectItem( json, ChannelConfigJson::CLASSIFICATION_CONFIG_TAG );
        if ( classificationConfigs && cJSON_IsArray( classificationConfigs ) )
        {
            uint8_t i    = 0;
            cJSON * item = nullptr;
            cJSON_ArrayForEach( item, classificationConfigs )
            {
                if ( i >= Channel::Config::CLASSIFICATION_CONFIG_COUNT ) break;
                fromJson( config.classificationConfigs[ i ], item );
                ++i;
            }
        }

        cJSON * tapTimeout = cJSON_GetObjectItem( json, ChannelConfigJson::TAP_TIMEOUT_TAG );
        if ( tapTimeout && cJSON_IsNumber( tapTimeout ) )
            config.tapTimeoutMs = static_cast< uint32_t >( tapTimeout->valuedouble );

        cJSON * tankCapacity = cJSON_GetObjectItem( json, ChannelConfigJson::TANK_CAPACITY_TAG );
        if ( tankCapacity && cJSON_IsNumber( tankCapacity ) )
            config.tankCapacity = static_cast< uint32_t >( tankCapacity->valuedouble );

        cJSON * cleaningVolumeThr = cJSON_GetObjectItem( json, ChannelConfigJson::CLEANING_VOLUME_THR_TAG );
        if ( cleaningVolumeThr && cJSON_IsNumber( cleaningVolumeThr ) )
            config.cleaningVolumeThr = static_cast< uint32_t >( cleaningVolumeThr->valuedouble );
    }

}    // namespace AsnPlus::Cloud

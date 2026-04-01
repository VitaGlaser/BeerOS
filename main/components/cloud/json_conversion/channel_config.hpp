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

    namespace BeerSizeConfigJson
    {
        static constexpr const char NAME_TAG[]       = "name";
        static constexpr const char MIN_VOLUME_TAG[] = "minVolume";
        static constexpr const char TYP_VOLUME_TAG[] = "typVolume";
        static constexpr const char MAX_VOLUME_TAG[] = "maxVolume";
    }    // namespace BeerSizeConfigJson

    namespace ChannelConfigJson
    {
        static constexpr const char TIMESTAMP_TAG[]           = "timestamp";
        static constexpr const char PORT_CONFIG_ID_TAG[]      = "portConfigId";
        static constexpr const char ENABLED_TAG[]             = "enabled";
        static constexpr const char FLOW_TYPE_TAG[]           = "flowType";
        static constexpr const char FLOW_CONFIG_TAG[]         = "flowConfig";
        static constexpr const char TEMP_TYPE_TAG[]           = "temperatureType";
        static constexpr const char TEMP_CONFIG_TAG[]         = "temperatureConfig";
        static constexpr const char PRESS_TYPE_TAG[]          = "pressureType";
        static constexpr const char PRESS_CONFIG_TAG[]        = "pressureConfig";
        static constexpr const char BEER_TYPE_ID_TAG[]        = "beerTypeId";
        static constexpr const char BEER_SIZES_TAG[]          = "beerSizes";
        static constexpr const char TAP_TIMEOUT_TAG[]         = "tapTimeoutMs";
        static constexpr const char TANK_CAPACITY_TAG[]       = "tankCapacity";
        static constexpr const char CLEANING_VOLUME_THR_TAG[] = "cleaningVolumeThr";
    }    // namespace ChannelConfigJson

    // MARK: toJson / fromJson — SensorConfig

    void toJson( Sensor::Config & config, cJSON * json )
    {
        cJSON_AddNumberToObject( json, SensorConfigJson::TIMESTAMP_TAG, static_cast< double >( config.timestamp ) );
        cJSON_AddBoolToObject( json, SensorConfigJson::ENABLED_TAG, config.enabled );
        cJSON_AddNumberToObject( json, SensorConfigJson::ID_TAG, static_cast< double >( config.id ) );
    }

    void fromJson( Sensor::Config & config, cJSON * json )
    {
        config.timestamp = static_cast< uint64_t >(
            cJSON_GetNumberValue( cJSON_GetObjectItem( json, SensorConfigJson::TIMESTAMP_TAG ) )
        );

        cJSON * enabled = cJSON_GetObjectItem( json, SensorConfigJson::ENABLED_TAG );
        if ( enabled && cJSON_IsBool( enabled ) ) config.enabled = cJSON_IsTrue( enabled );

        cJSON * id = cJSON_GetObjectItem( json, SensorConfigJson::ID_TAG );
        if ( id && cJSON_IsNumber( id ) ) config.id = static_cast< uint64_t >( id->valuedouble );
    }

    // MARK: toJson / fromJson — BeerSizeConfig

    void toJson( Channel::Config::BeerSizeConfig & config, cJSON * json )
    {
        cJSON_AddStringToObject( json, BeerSizeConfigJson::NAME_TAG, config.name );
        cJSON_AddNumberToObject( json, BeerSizeConfigJson::MIN_VOLUME_TAG, config.minVolume );
        cJSON_AddNumberToObject( json, BeerSizeConfigJson::TYP_VOLUME_TAG, config.typVolume );
        cJSON_AddNumberToObject( json, BeerSizeConfigJson::MAX_VOLUME_TAG, config.maxVolume );
    }

    void fromJson( Channel::Config::BeerSizeConfig & config, cJSON * json )
    {
        cJSON * name = cJSON_GetObjectItem( json, BeerSizeConfigJson::NAME_TAG );
        if ( name && cJSON_IsString( name ) )
            strncpy( config.name, name->valuestring, Channel::Config::BeerSizeConfig::NAME_LENGTH - 1 );

        cJSON * minVol = cJSON_GetObjectItem( json, BeerSizeConfigJson::MIN_VOLUME_TAG );
        if ( minVol && cJSON_IsNumber( minVol ) )
            config.minVolume = static_cast< uint32_t >( minVol->valuedouble );

        cJSON * typVol = cJSON_GetObjectItem( json, BeerSizeConfigJson::TYP_VOLUME_TAG );
        if ( typVol && cJSON_IsNumber( typVol ) )
            config.typVolume = static_cast< uint32_t >( typVol->valuedouble );

        cJSON * maxVol = cJSON_GetObjectItem( json, BeerSizeConfigJson::MAX_VOLUME_TAG );
        if ( maxVol && cJSON_IsNumber( maxVol ) )
            config.maxVolume = static_cast< uint32_t >( maxVol->valuedouble );
    }

    // MARK: toJson / fromJson — ChannelConfig

    void toJson( ChannelConfig & config, cJSON * json )
    {
        cJSON_AddNumberToObject( json, ChannelConfigJson::TIMESTAMP_TAG, static_cast< double >( config.timestamp ) );
        cJSON_AddStringToObject( json, ChannelConfigJson::PORT_CONFIG_ID_TAG, config.portConfigId );
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

        cJSON_AddNumberToObject( json, ChannelConfigJson::BEER_TYPE_ID_TAG, config.beerTypeId );

        cJSON * beerSizesArr = cJSON_CreateArray();
        for ( uint8_t i = 0; i < Channel::Config::BEER_SIZE_COUNT; ++i )
        {
            cJSON * sizeJson = cJSON_CreateObject();
            toJson( config.beerSizes[ i ], sizeJson );
            cJSON_AddItemToArray( beerSizesArr, sizeJson );
        }
        cJSON_AddItemToObject( json, ChannelConfigJson::BEER_SIZES_TAG, beerSizesArr );

        cJSON_AddNumberToObject( json, ChannelConfigJson::TAP_TIMEOUT_TAG, config.tapTimeoutMs );
        cJSON_AddNumberToObject( json, ChannelConfigJson::TANK_CAPACITY_TAG, config.tankCapacity );
        cJSON_AddNumberToObject( json, ChannelConfigJson::CLEANING_VOLUME_THR_TAG, config.cleaningVolumeThr );
    }

    void fromJson( ChannelConfig & config, cJSON * json )
    {
        config.timestamp = static_cast< uint64_t >(
            cJSON_GetNumberValue( cJSON_GetObjectItem( json, ChannelConfigJson::TIMESTAMP_TAG ) )
        );

        cJSON * portConfigId = cJSON_GetObjectItem( json, ChannelConfigJson::PORT_CONFIG_ID_TAG );
        if ( portConfigId && cJSON_IsString( portConfigId ) )
            strncpy( config.portConfigId, portConfigId->valuestring, Channel::Config::PORT_CONFIG_ID_LENGTH - 1 );

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

        cJSON * beerTypeId = cJSON_GetObjectItem( json, ChannelConfigJson::BEER_TYPE_ID_TAG );
        if ( beerTypeId && cJSON_IsNumber( beerTypeId ) )
            config.beerTypeId = static_cast< uint16_t >( beerTypeId->valuedouble );

        cJSON * beerSizesArr = cJSON_GetObjectItem( json, ChannelConfigJson::BEER_SIZES_TAG );
        if ( beerSizesArr && cJSON_IsArray( beerSizesArr ) )
        {
            uint8_t i    = 0;
            cJSON * item = nullptr;
            cJSON_ArrayForEach( item, beerSizesArr )
            {
                if ( i >= Channel::Config::BEER_SIZE_COUNT ) break;
                fromJson( config.beerSizes[ i ], item );
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

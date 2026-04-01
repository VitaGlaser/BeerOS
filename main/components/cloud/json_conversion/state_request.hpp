#pragma once

#include "cJSON.h"

#include "asn/asn-core/types.hpp"
#include "asn/asn-hal/common/common_structs.hpp"

#include "components/connection/structs.hpp"

namespace AsnPlus::Cloud
{
    // MARK: JSON tags

    namespace BeerSizeTypeStateJson
    {
        static constexpr const char COUNT_TAG[]           = "count";
        static constexpr const char VOLUME_TAG[]          = "volume";
        static constexpr const char QUALITY_TAG[]         = "quality";
        static constexpr const char UNDER_LIMIT_TAG[]     = "underLimitCount";
        static constexpr const char OVER_LIMIT_TAG[]      = "overLimitCount";
    }    // namespace BeerSizeTypeStateJson

    namespace UnrecognizedJson
    {
        static constexpr const char COUNT_TAG[]  = "count";
        static constexpr const char VOLUME_TAG[] = "volume";
    }    // namespace UnrecognizedJson

    namespace ChannelStateJson
    {
        static constexpr const char STATUS_TAG[]          = "status";
        static constexpr const char TANK_LEVEL_TAG[]      = "tankLevel";
        static constexpr const char TEMPERATURE_TAG[]     = "temperature";
        static constexpr const char PRESSURE_TAG[]        = "pressure";
        static constexpr const char CONDUCTIVITY_TAG[]    = "conductivity";
        static constexpr const char UNRECOGNIZED_TAG[]    = "unrecognized";
        static constexpr const char SIZE_TYPE_STATE_TAG[] = "sizeTypeState";
    }    // namespace ChannelStateJson

    namespace ConnectionStateJson
    {
        static constexpr const char TIMESTAMP_TAG[]  = "timestamp";
        static constexpr const char ETH_STATUS_TAG[] = "ethStatus";
        static constexpr const char WIFI_STATUS_TAG[] = "wifiStatus";
        static constexpr const char LTE_STATUS_TAG[] = "lteStatus";
    }    // namespace ConnectionStateJson

    namespace StateRequestJson
    {
        static constexpr const char TIMESTAMP_TAG[]        = "timestamp";
        static constexpr const char STATUS_TAG[]           = "status";
        static constexpr const char RUNTIME_TAG[]          = "runtime";
        static constexpr const char CONNECTION_STATE_TAG[] = "connectionState";
        static constexpr const char CHANNELS_STATE_TAG[]   = "channelsState";
    }    // namespace StateRequestJson

    // MARK: Structs

    struct BeerSizeTypeState
    {
        uint32_t count           = 0;
        uint32_t volume          = 0;
        uint32_t quality         = 0;
        uint32_t underLimitCount = 0;
        uint32_t overLimitCount  = 0;
    };

    struct Unrecognized
    {
        uint32_t count  = 0;
        uint32_t volume = 0;
    };

    enum class ChannelStatus : uint8_t
    {
        UNKNOWN = 0,
        TBD
    };

    struct ChannelState
    {
        static constexpr uint8_t BEER_SIZE_COUNT = 8;

        ChannelStatus     status                               = ChannelStatus::UNKNOWN;
        uint64_t          tankLevel                           = 0;
        uint64_t          temperature                         = 0;
        uint64_t          pressure                            = 0;
        uint64_t          conductivity                        = 0;
        Unrecognized      unrecognized                        {};
        BeerSizeTypeState sizeTypeState[ BEER_SIZE_COUNT ]    {};
    };

    struct ConnectionState
    {
        uint64_t          timestamp  = 0;
        Connection::Value ethStatus  = Connection::Value::UNKNOWN;
        Connection::Value wifiStatus = Connection::Value::UNKNOWN;
        Connection::Value lteStatus  = Connection::Value::UNKNOWN;
    };

    struct StateRequest
    {
        static constexpr uint8_t CHANNEL_COUNT = 4;

        uint64_t        timestamp                          = 0;
        AsnPlus::Status status                             = AsnPlus::Status::UNKNOWN;
        uint64_t        runtime                            = 0;
        ConnectionState connectionState                    {};
        ChannelState    channelsState[ CHANNEL_COUNT ]     {};
    };

    // MARK: toJson

    void toJson( BeerSizeTypeState & state, cJSON * json )
    {
        cJSON_AddNumberToObject( json, BeerSizeTypeStateJson::COUNT_TAG, state.count );
        cJSON_AddNumberToObject( json, BeerSizeTypeStateJson::VOLUME_TAG, state.volume );
        cJSON_AddNumberToObject( json, BeerSizeTypeStateJson::QUALITY_TAG, state.quality );
        cJSON_AddNumberToObject( json, BeerSizeTypeStateJson::UNDER_LIMIT_TAG, state.underLimitCount );
        cJSON_AddNumberToObject( json, BeerSizeTypeStateJson::OVER_LIMIT_TAG, state.overLimitCount );
    }

    void toJson( Unrecognized & state, cJSON * json )
    {
        cJSON_AddNumberToObject( json, UnrecognizedJson::COUNT_TAG, state.count );
        cJSON_AddNumberToObject( json, UnrecognizedJson::VOLUME_TAG, state.volume );
    }

    void toJson( ChannelState & state, cJSON * json )
    {
        cJSON_AddNumberToObject( json, ChannelStateJson::STATUS_TAG, static_cast< uint8_t >( state.status ) );
        cJSON_AddNumberToObject( json, ChannelStateJson::TANK_LEVEL_TAG, static_cast< double >( state.tankLevel ) );
        cJSON_AddNumberToObject( json, ChannelStateJson::TEMPERATURE_TAG, static_cast< double >( state.temperature ) );
        cJSON_AddNumberToObject( json, ChannelStateJson::PRESSURE_TAG, static_cast< double >( state.pressure ) );
        cJSON_AddNumberToObject( json, ChannelStateJson::CONDUCTIVITY_TAG, static_cast< double >( state.conductivity ) );

        cJSON * unrecJson = cJSON_CreateObject();
        toJson( state.unrecognized, unrecJson );
        cJSON_AddItemToObject( json, ChannelStateJson::UNRECOGNIZED_TAG, unrecJson );

        cJSON * sizeArr = cJSON_CreateArray();
        for ( uint8_t i = 0; i < ChannelState::BEER_SIZE_COUNT; ++i )
        {
            cJSON * sizeJson = cJSON_CreateObject();
            toJson( state.sizeTypeState[ i ], sizeJson );
            cJSON_AddItemToArray( sizeArr, sizeJson );
        }
        cJSON_AddItemToObject( json, ChannelStateJson::SIZE_TYPE_STATE_TAG, sizeArr );
    }

    void toJson( ConnectionState & state, cJSON * json )
    {
        cJSON_AddNumberToObject( json, ConnectionStateJson::TIMESTAMP_TAG, static_cast< double >( state.timestamp ) );
        cJSON_AddNumberToObject(
            json, ConnectionStateJson::ETH_STATUS_TAG, static_cast< uint8_t >( state.ethStatus )
        );
        cJSON_AddNumberToObject(
            json, ConnectionStateJson::WIFI_STATUS_TAG, static_cast< uint8_t >( state.wifiStatus )
        );
        cJSON_AddNumberToObject(
            json, ConnectionStateJson::LTE_STATUS_TAG, static_cast< uint8_t >( state.lteStatus )
        );
    }

    void toJson( StateRequest & request, cJSON * json )
    {
        cJSON_AddNumberToObject( json, StateRequestJson::TIMESTAMP_TAG, static_cast< double >( request.timestamp ) );
        cJSON_AddNumberToObject( json, StateRequestJson::STATUS_TAG, static_cast< uint8_t >( request.status ) );
        cJSON_AddNumberToObject( json, StateRequestJson::RUNTIME_TAG, static_cast< double >( request.runtime ) );

        cJSON * connJson = cJSON_CreateObject();
        toJson( request.connectionState, connJson );
        cJSON_AddItemToObject( json, StateRequestJson::CONNECTION_STATE_TAG, connJson );

        cJSON * channelsArr = cJSON_CreateArray();
        for ( uint8_t i = 0; i < StateRequest::CHANNEL_COUNT; ++i )
        {
            cJSON * channelJson = cJSON_CreateObject();
            toJson( request.channelsState[ i ], channelJson );
            cJSON_AddItemToArray( channelsArr, channelJson );
        }
        cJSON_AddItemToObject( json, StateRequestJson::CHANNELS_STATE_TAG, channelsArr );
    }

}    // namespace AsnPlus::Cloud

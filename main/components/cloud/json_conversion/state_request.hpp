#pragma once

#include "cJSON.h"

#include "asn/asn-core/types.hpp"
#include "asn/asn-hal/include/common/common_structs.hpp"
#include "asn/asn-hal/include/common/identification_structs.hpp"

#include "components/connection/structs.hpp"

namespace AsnPlus::Cloud
{
    // MARK: JSON tags

    namespace ClassificationStateJson
    {
        static constexpr const char COUNT_TAG[]       = "count";
        static constexpr const char VOLUME_TAG[]      = "volume";
        static constexpr const char QUALITY_TAG[]     = "quality";
        static constexpr const char UNDER_LIMIT_TAG[] = "underLimitCount";
        static constexpr const char OVER_LIMIT_TAG[]  = "overLimitCount";
    }    // namespace ClassificationStateJson

    namespace UnrecognizedJson
    {
        static constexpr const char COUNT_TAG[]  = "count";
        static constexpr const char VOLUME_TAG[] = "volume";
    }    // namespace UnrecognizedJson

    namespace ChannelStateJson
    {
        static constexpr const char STATUS_TAG[]               = "status";
        static constexpr const char TANK_LEVEL_TAG[]           = "tankLevel";
        static constexpr const char TEMPERATURE_TAG[]          = "temperature";
        static constexpr const char PRESSURE_TAG[]             = "pressure";
        static constexpr const char FLOW_TAG[]                 = "flow";
        static constexpr const char CONDUCTIVITY_TAG[]         = "conductivity";
        static constexpr const char UNRECOGNIZED_TAG[]         = "unrecognized";
        static constexpr const char CLASSIFICATION_STATE_TAG[] = "classificationState";
    }    // namespace ChannelStateJson

    namespace ConnectionStateJson
    {
        static constexpr const char TIMESTAMP_TAG[]  = "timestamp";
        static constexpr const char ETH_STATE_TAG[]  = "ethStatus";
        static constexpr const char WIFI_STATE_TAG[] = "wifiStatus";
        static constexpr const char LTE_STATE_TAG[]  = "lteStatus";
    }    // namespace ConnectionStateJson

    namespace FirmwareInfoJson
    {
        static constexpr const char VERSION_TAG[]            = "version";
        static constexpr const char DATA_MODEL_VERSION_TAG[] = "dataModelVersion";
    }    // namespace FirmwareInfoJson

    namespace ManufactureInfoJson
    {
        static constexpr const char CTS_TAG[]         = "cts";
        static constexpr const char UID_TAG[]         = "uid";
        static constexpr const char ENVIRONMENT_TAG[] = "environment";
    }    // namespace ManufactureInfoJson

    namespace StateRequestJson
    {
        static constexpr const char TIMESTAMP_TAG[]        = "timestamp";
        static constexpr const char STATUS_TAG[]           = "status";
        static constexpr const char RUNTIME_TAG[]          = "runtime";
        static constexpr const char FIRMWARE_INFO_TAG[]    = "firmwareInfo";
        static constexpr const char MANUFACTURE_INFO_TAG[] = "manufactureInfo";
        static constexpr const char CONNECTION_STATE_TAG[] = "connectionState";
        static constexpr const char CHANNELS_STATE_TAG[]   = "channelsState";
    }    // namespace StateRequestJson

    // MARK: Structs

    struct ClassificationState
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

    struct ChannelState
    {
        static constexpr uint8_t CLASSIFICATION_COUNT = 8;

        AsnPlus::Status     status                    = AsnPlus::Status::UNKNOWN;
        uint32_t            tankLevel                 = 0;
        uint16_t            temperature               = 0;
        uint16_t            pressure                  = 0;
        uint16_t            flow                      = 0;
        uint16_t            conductivity              = 0;
        Unrecognized        unrecognized {};
        ClassificationState classificationState[ CLASSIFICATION_COUNT ] {};
    };

    struct ConnectionState
    {
        uint64_t          timestamp  = 0;
        Connection::State ethStatus  = Connection::State::UNKNOWN;
        Connection::State wifiStatus = Connection::State::UNKNOWN;
        Connection::State lteStatus  = Connection::State::UNKNOWN;
    };

    struct StateRequest
    {
        static constexpr uint8_t CHANNEL_COUNT = 4;

        uint64_t                 timestamp     = 0;
        AsnPlus::Status          status        = AsnPlus::Status::UNKNOWN;
        uint32_t                 runtime       = 0;
        AsnPlus::FirmwareInfo    firmwareInfo {};
        AsnPlus::ManufactureInfo manufactureInfo {};
        ConnectionState          connectionState {};
        ChannelState             channelsState[ CHANNEL_COUNT ] {};
    };

    // MARK: toJson

    void toJson( AsnPlus::FirmwareInfo & info, cJSON * json )
    {
        cJSON_AddNumberToObject( json, FirmwareInfoJson::VERSION_TAG, info.version );
        cJSON_AddNumberToObject( json, FirmwareInfoJson::DATA_MODEL_VERSION_TAG, info.dataModelVersion );
    }

    void toJson( AsnPlus::ManufactureInfo & info, cJSON * json )
    {
        cJSON_AddNumberToObject( json, ManufactureInfoJson::CTS_TAG, static_cast< double >( info.cts ) );

        char uid_str[ AsnPlus::ManufactureInfo::UID_LENGTH + 1 ];
        memcpy( uid_str, info.uid, AsnPlus::ManufactureInfo::UID_LENGTH );
        uid_str[ AsnPlus::ManufactureInfo::UID_LENGTH ] = '\0';
        cJSON_AddStringToObject( json, ManufactureInfoJson::UID_TAG, uid_str );

        cJSON_AddNumberToObject(
            json, ManufactureInfoJson::ENVIRONMENT_TAG, static_cast< uint8_t >( info.environment )
        );
    }

    void toJson( ClassificationState & state, cJSON * json )
    {
        cJSON_AddNumberToObject( json, ClassificationStateJson::COUNT_TAG, state.count );
        cJSON_AddNumberToObject( json, ClassificationStateJson::VOLUME_TAG, state.volume );
        cJSON_AddNumberToObject( json, ClassificationStateJson::QUALITY_TAG, state.quality );
        cJSON_AddNumberToObject( json, ClassificationStateJson::UNDER_LIMIT_TAG, state.underLimitCount );
        cJSON_AddNumberToObject( json, ClassificationStateJson::OVER_LIMIT_TAG, state.overLimitCount );
    }

    void toJson( Unrecognized & state, cJSON * json )
    {
        cJSON_AddNumberToObject( json, UnrecognizedJson::COUNT_TAG, state.count );
        cJSON_AddNumberToObject( json, UnrecognizedJson::VOLUME_TAG, state.volume );
    }

    void toJson( ChannelState & state, cJSON * json )
    {
        cJSON_AddNumberToObject( json, ChannelStateJson::STATUS_TAG, static_cast< uint8_t >( state.status ) );
        cJSON_AddNumberToObject( json, ChannelStateJson::TANK_LEVEL_TAG, state.tankLevel );
        cJSON_AddNumberToObject( json, ChannelStateJson::TEMPERATURE_TAG, state.temperature );
        cJSON_AddNumberToObject( json, ChannelStateJson::PRESSURE_TAG, state.pressure );
        cJSON_AddNumberToObject( json, ChannelStateJson::FLOW_TAG, state.flow );
        cJSON_AddNumberToObject( json, ChannelStateJson::CONDUCTIVITY_TAG, state.conductivity );

        cJSON * unrecJson = cJSON_CreateObject();
        toJson( state.unrecognized, unrecJson );
        cJSON_AddItemToObject( json, ChannelStateJson::UNRECOGNIZED_TAG, unrecJson );

        cJSON * classArr = cJSON_CreateArray();
        for ( uint8_t i = 0; i < ChannelState::CLASSIFICATION_COUNT; ++i )
        {
            cJSON * classJson = cJSON_CreateObject();
            toJson( state.classificationState[ i ], classJson );
            cJSON_AddItemToArray( classArr, classJson );
        }
        cJSON_AddItemToObject( json, ChannelStateJson::CLASSIFICATION_STATE_TAG, classArr );
    }

    void toJson( ConnectionState & state, cJSON * json )
    {
        cJSON_AddNumberToObject( json, ConnectionStateJson::TIMESTAMP_TAG, static_cast< double >( state.timestamp ) );
        cJSON_AddNumberToObject( json, ConnectionStateJson::ETH_STATE_TAG, static_cast< uint8_t >( state.ethStatus ) );
        cJSON_AddNumberToObject(
            json, ConnectionStateJson::WIFI_STATE_TAG, static_cast< uint8_t >( state.wifiStatus )
        );
        cJSON_AddNumberToObject( json, ConnectionStateJson::LTE_STATE_TAG, static_cast< uint8_t >( state.lteStatus ) );
    }

    void toJson( StateRequest & request, cJSON * json )
    {
        cJSON_AddNumberToObject( json, StateRequestJson::TIMESTAMP_TAG, static_cast< double >( request.timestamp ) );
        cJSON_AddNumberToObject( json, StateRequestJson::STATUS_TAG, static_cast< uint8_t >( request.status ) );
        cJSON_AddNumberToObject( json, StateRequestJson::RUNTIME_TAG, request.runtime );

        cJSON * fwInfoJson = cJSON_CreateObject();
        toJson( request.firmwareInfo, fwInfoJson );
        cJSON_AddItemToObject( json, StateRequestJson::FIRMWARE_INFO_TAG, fwInfoJson );

        cJSON * mfInfoJson = cJSON_CreateObject();
        toJson( request.manufactureInfo, mfInfoJson );
        cJSON_AddItemToObject( json, StateRequestJson::MANUFACTURE_INFO_TAG, mfInfoJson );

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

#pragma once

#include "program/config.hpp"

#include <cmath>
#include <cstring>

#include "cJSON.h"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/vector.hpp"

#include "firestore_request.hpp"

#include "components/cloud/json_conversion/state_request.hpp"

namespace AsnPlus::Cloud
{
    class UnitStatusRequest : public IFirestoreRequest
    {
    public:
        using Config = IFirestoreRequest::Config;

        UnitStatusRequest(
            StateRequest &       stateRequest,
            Config &             config,
            IVector< uint8_t > & requestBuffer,
            IVector< uint8_t > & responseBuffer
        ) :
            IFirestoreRequest( config ),
            _stateRequest( stateRequest ),
            _config( config ),
            _requestBuffer( requestBuffer ),
            _responseBuffer( responseBuffer )
        {
        }

        bool initialize() override
        {
            snprintf( _url, sizeof( _url ), "%s", _config.baseUrl );
            return true;
        }

        bool isOtaAvailable() const { return _otaAvailable; }
        bool isOtaMandatory() const { return _otaMandatory; }
        const char * getOtaVersion() const { return _otaVersion; }
        const char * getOtaUrl() const { return _otaUrl; }
        void setReportedOtaStatus( const char * status, const char * targetVersion = nullptr )
        {
            if ( status && status[ 0 ] != '\0' )
                StringExt( _reportedOtaStatus, _reportedOtaStatus, sizeof( _reportedOtaStatus ) ).assign( status );

            if ( targetVersion && targetVersion[ 0 ] != '\0' )
                StringExt( _reportedOtaTargetVersion, _reportedOtaTargetVersion, sizeof( _reportedOtaTargetVersion ) )
                    .assign( targetVersion );
            else
                _reportedOtaTargetVersion[ 0 ] = '\0';
        }

        bool send() override
        {
            cJSON * requestJson = cJSON_CreateObject();
            if ( ! requestJson )
            {
                Log::error( "Failed to allocate unit status JSON" );
                return false;
            }

            _buildJson( requestJson );

            char * body = cJSON_PrintUnformatted( requestJson );
            cJSON_Delete( requestJson );

            if ( ! body )
            {
                Log::error( "Failed to serialize unit status JSON" );
                return false;
            }

            const size_t bodyLen = strlen( body );
            if ( bodyLen > _requestBuffer.capacity() )
            {
                Log::error(
                    "Unit status body too large (%u > %u)",
                    static_cast< unsigned >( bodyLen ),
                    static_cast< unsigned >( _requestBuffer.capacity() )
                );
                cJSON_free( body );
                return false;
            }

            const auto * bodyData = reinterpret_cast< const uint8_t * >( body );
            _requestBuffer.assign( bodyData, bodyData + bodyLen );
            cJSON_free( body );

            _responseBuffer.clear();
            Https::Request  req  { .method = Https::Method::POST, .payload = &_requestBuffer };
            Https::Response resp { .response = &_responseBuffer };

            uint16_t status = request( &req, &resp );
            if ( status < 200 || status >= 300 )
            {
                Log::warn( "Unit status request (%s) failed with status code %u", _url, status );
                return false;
            }

            _parseResponse();

            Log::debug( "Unit status request (%s) succeeded with status code %u", _url, status );
            return true;
        }

    private:
        static constexpr const char TAG[] = "UnitStatusRequest";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_CLOUD_REQUESTS, TAG >;

        StateRequest &       _stateRequest;
        Config &             _config;
        IVector< uint8_t > & _requestBuffer;
        IVector< uint8_t > & _responseBuffer;
        bool                 _otaAvailable = false;
        bool                 _otaMandatory = false;
        char                 _otaVersion[ 32 ] {};
        char                 _otaUrl[ 192 ] {};
        char                 _reportedOtaStatus[ 16 ] { 'i', 'd', 'l', 'e', '\0' };
        char                 _reportedOtaTargetVersion[ 32 ] {};

        void _parseResponse()
        {
            if ( _responseBuffer.empty() )
            {
                _otaAvailable = false;
                _otaMandatory = false;
                _otaVersion[ 0 ] = '\0';
                _otaUrl[ 0 ] = '\0';
                return;
            }

            cJSON * responseJson =
                cJSON_ParseWithLength( reinterpret_cast< const char * >( _responseBuffer.data() ), _responseBuffer.size() );

            if ( ! responseJson )
            {
                Log::warn( "Unit status response is not valid JSON" );
                return;
            }

            cJSON * ota       = cJSON_GetObjectItem( responseJson, "ota" );
            cJSON * available = ota ? cJSON_GetObjectItem( ota, "available" ) : nullptr;
            if ( available && cJSON_IsBool( available ) )
            {
                _otaAvailable = cJSON_IsTrue( available );
            }

            cJSON * mandatory = ota ? cJSON_GetObjectItem( ota, "mandatory" ) : nullptr;
            _otaMandatory     = ( mandatory && cJSON_IsBool( mandatory ) ) ? cJSON_IsTrue( mandatory ) : false;

            cJSON * version = ota ? cJSON_GetObjectItem( ota, "version" ) : nullptr;
            if ( version && cJSON_IsString( version ) && version->valuestring )
                StringExt( _otaVersion, _otaVersion, sizeof( _otaVersion ) ).assign( version->valuestring );
            else
                _otaVersion[ 0 ] = '\0';

            cJSON * url = ota ? cJSON_GetObjectItem( ota, "url" ) : nullptr;
            if ( url && cJSON_IsString( url ) && url->valuestring )
                StringExt( _otaUrl, _otaUrl, sizeof( _otaUrl ) ).assign( url->valuestring );
            else
                _otaUrl[ 0 ] = '\0';

            Log::info(
                "OTA availability=%s mandatory=%s version=%s url=%s",
                _otaAvailable ? "true" : "false",
                _otaMandatory ? "true" : "false",
                _otaVersion[ 0 ] ? _otaVersion : "",
                _otaUrl[ 0 ] ? _otaUrl : ""
            );

            cJSON_Delete( responseJson );
        }

        void _buildJson( cJSON * json )
        {
            char uidStr[ ManufactureInfo::UID_LENGTH + 1 ] = {};
            memcpy( uidStr, _stateRequest.manufactureInfo.uid, ManufactureInfo::UID_LENGTH );
            uidStr[ ManufactureInfo::UID_LENGTH ] = '\0';

            cJSON_AddNumberToObject( json, "timestamp", static_cast< double >( _stateRequest.timestamp ) );
            cJSON_AddStringToObject( json, "uid", uidStr );
            cJSON_AddNumberToObject( json, "firmware", _stateRequest.firmwareInfo.version );
            cJSON_AddNumberToObject( json, "runtime", _stateRequest.runtime );
            const double roundedBatteryVoltage =
                std::round( static_cast< double >( _stateRequest.batteryVoltage ) * 1000.0 ) / 1000.0;
            cJSON_AddNumberToObject( json, "batt", roundedBatteryVoltage );
            cJSON_AddBoolToObject( json, "ACOK", _stateRequest.chargerAcOk );
            cJSON_AddBoolToObject( json, "CHGOK", _stateRequest.chargerChgOk );
            cJSON_AddStringToObject( json, "otaStatus", _reportedOtaStatus );
            if ( _reportedOtaTargetVersion[ 0 ] != '\0' )
            {
                cJSON_AddStringToObject( json, "otaTargetVersion", _reportedOtaTargetVersion );
            }
        }
    };
}    // namespace AsnPlus::Cloud

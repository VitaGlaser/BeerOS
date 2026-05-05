#pragma once

#include "program/config.hpp"

#include "cJSON.h"

#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/vector.hpp"

#include "firestore_request.hpp"

#include "components/cloud/json_conversion/state_request.hpp"
#include "components/cloud/json_conversion/state_response.hpp"

namespace AsnPlus::Cloud
{
    class StatePostRequest : public IFirestoreRequest
    {
    public:
        StatePostRequest(
            StateRequest &              request,
            StateResponse &             response,
            IFirestoreRequest::Config & config,
            IVector< uint8_t > &        requestBuffer,
            IVector< uint8_t > &        responseBuffer,
            Delegate< void() >          onStateResponse
        ) :
            IFirestoreRequest( config ),
            _config( config ),
            _request( request ),
            _response( response ),
            _requestBuffer( requestBuffer ),
            _responseBuffer( responseBuffer ),
            _onStateResponse( onStateResponse )
        {
        }

        bool initialize() override
        {
            formatUrl(
                _url,
                sizeof( _url ),
                _config.baseUrl,
                _config.moduleUrl,
                _config.uid,
                ManufactureInfo::UID_LENGTH,
                nullptr
            );

            return true;
        }

        bool send() override
        {
            cJSON * requestJson = cJSON_CreateObject();
            if ( ! requestJson )
            {
                Log::error( "Failed to allocate request JSON" );
                return false;
            }

            toJson( _request, requestJson );

            char * body = cJSON_PrintUnformatted( requestJson );
            cJSON_Delete( requestJson );

            if ( ! body )
            {
                Log::error( "Failed to serialize request JSON" );
                return false;
            }

            const size_t bodyLen  = strlen( body );
            const size_t copyLen  = ( bodyLen < _requestBuffer.capacity() ) ? bodyLen : _requestBuffer.capacity();
            const auto * bodyData = reinterpret_cast< const uint8_t * >( body );
            _requestBuffer.assign( bodyData, bodyData + copyLen );
            cJSON_free( body );

            _responseBuffer.clear();
            Https::Request  req  { .method = Https::Method::POST, .payload = &_requestBuffer };
            Https::Response resp { .response = &_responseBuffer };

            uint16_t status = request( &req, &resp );

            if ( status != 200 )
            {
                Log::error( "Request (%s) failed with status code %d", _url, status );
                return false;
            }

            if ( _responseBuffer.empty() ) return true;

            Log::info( "Request (%s) succeeded with status code %d", _url, status );

            char responseLabel[ 64 ];
            snprintf( responseLabel, sizeof( responseLabel ), "Response [%s]", _config.moduleUrl );
            Log::hexdump( responseLabel, _responseBuffer.data(), _responseBuffer.size() );

            cJSON * responseJson = cJSON_Parse( reinterpret_cast< char * >( _responseBuffer.data() ) );
            if ( ! responseJson )
            {
                Log::error( "JSON parsing failed (%s)", _url );
                return false;
            }

            fromJson( _response, responseJson );
            cJSON_Delete( responseJson );

            _onStateResponse();
            return true;
        }

    private:
        static constexpr const char TAG[] = "StatePostRequest";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_CLOUD_REQUESTS, TAG >;

        IFirestoreRequest::Config & _config;

        StateRequest &       _request;
        StateResponse &      _response;
        IVector< uint8_t > & _requestBuffer;
        IVector< uint8_t > & _responseBuffer;

        Delegate< void() > _onStateResponse;
    };
}    // namespace AsnPlus::Cloud

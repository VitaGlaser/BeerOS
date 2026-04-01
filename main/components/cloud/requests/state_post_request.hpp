#pragma once

#include "program/config.hpp"

#include "cJSON.h"

#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/vector.hpp"

#include "asn/asn-esp32-wifi/https/client/ifirestore_request.hpp"

#include "asn/asn-hal/common/common_structs.hpp"

#include "components/cloud/json_conversion/state_request.hpp"
#include "components/cloud/json_conversion/state_response.hpp"

namespace AsnPlus::Cloud
{
    class StatePostRequest : public Esp32::Https::IFirestoreRequest
    {
    public:
        StatePostRequest(
            StateRequest &                            request,
            StateResponse &                           response,
            Esp32::Https::IFirestoreRequest::Config & config,
            IVector< uint8_t > &                      responseBuffer,
            Delegate< void() >                        onStateResponse
        ) :
            IFirestoreRequest( config ),
            _config( config ),
            _request( request ),
            _response( response ),
            _responseBuffer( responseBuffer ),
            _onStateResponse( onStateResponse )
        {
            _config.method = Esp32::Https::IClient::Method::POST;
        }

        bool initialize() override
        {
            formatUrl(
                _url,
                sizeof( _url ),
                _config.baseUrl,
                _config.moduleUrl,
                _config.uuid,
                ManufactureInfo::UID_LENGTH,
                NULL
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

            _responseBuffer.resize( _responseBuffer.capacity() );
            uint32_t responseLen = static_cast< uint32_t >( _responseBuffer.size() );

            uint32_t ret = request(
                nullptr,
                0,
                reinterpret_cast< const uint8_t * >( body ),
                static_cast< uint32_t >( strlen( body ) ),
                _responseBuffer.data(),
                responseLen
            );

            cJSON_free( body );

            if ( ret != 200 )
            {
                Log::error( "Request (%s) failed with status code %d", _url, ret );
                return false;
            }

            if ( responseLen == 0 ) return true;

            char response_label[ 64 ];
            snprintf( response_label, sizeof( response_label ), "Response [%s]", _config.moduleUrl );
            Log::hexdump( response_label, _responseBuffer.data(), responseLen );

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

        Esp32::Https::IFirestoreRequest::Config & _config;

        StateRequest &       _request;
        StateResponse &      _response;
        IVector< uint8_t > & _responseBuffer;

        Delegate< void() > _onStateResponse;
    };
}    // namespace AsnPlus::Cloud

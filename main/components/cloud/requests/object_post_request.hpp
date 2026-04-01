#pragma once

#include "program/config.hpp"

#include "cJSON.h"

#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/vector.hpp"

#include "asn/asn-esp32-wifi/https/client/ifirestore_request.hpp"

#include "asn/asn-hal/common/common_structs.hpp"

namespace AsnPlus::Cloud
{
    template< typename REQUEST, typename RESPONSE = REQUEST >
    class ObjectPostRequest : public Esp32::Https::IFirestoreRequest
    {
    public:
        ObjectPostRequest(
            REQUEST &                                 request,
            RESPONSE &                                response,
            Esp32::Https::IFirestoreRequest::Config & config,
            IVector< uint8_t > &                      buffer,
            IVector< uint8_t > &                      responseBuffer,
            Delegate< void() >                        onUpdate
        ) :
            IFirestoreRequest( config ),
            _config( config ),
            _request( request ),
            _response( response ),
            _buffer( buffer ),
            _responseBuffer( responseBuffer ),
            _onUpdate( onUpdate )
        {
            _config.method = Esp32::Https::IClient::Method::POST;
        }

        bool initialize() override
        {
            if ( _buffer.capacity() == 0 ) return false;

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
            if ( _buffer.capacity() == 0 ) return false;

            cJSON * root = cJSON_CreateObject();
            if ( ! root )
            {
                Log::error( "Failed to create root JSON object (%s)", _url );
                return false;
            }

            _request.toJson( root );

            _buffer.clear();

            if ( ! cJSON_PrintPreallocated(
                     root, reinterpret_cast< char * >( _buffer.data() ), static_cast< int >( _buffer.capacity() ), false
                 ) )
            {
                Log::error( "JSON serialization failed (%s)", _url );
                cJSON_Delete( root );
                return false;
            }

            const uint32_t requestLen = strlen( reinterpret_cast< char * >( _buffer.data() ) );
            char           request_label[ 64 ];
            snprintf( request_label, sizeof( request_label ), "Request [%s]", _config.moduleUrl );
            Log::hexdump( request_label, _buffer.data(), requestLen );
            cJSON_Delete( root );

            _responseBuffer.resize( _responseBuffer.capacity() );
            uint32_t responseLen = static_cast< uint32_t >( _responseBuffer.size() );

            uint32_t ret = request( nullptr, 0, _buffer.data(), requestLen, _responseBuffer.data(), responseLen );

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

            const uint64_t old_timestamp = _response.timestamp;
            _response.fromJson( responseJson );
            if ( _response.timestamp != old_timestamp )
            {
                _onUpdate();
            }

            cJSON_Delete( responseJson );
            return true;
        }

    private:
        static constexpr const char TAG[] = "ObjectPostRequest";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_CLOUD_REQUESTS, TAG >;

        Esp32::Https::IFirestoreRequest::Config & _config;

        REQUEST &            _request;
        RESPONSE &           _response;
        IVector< uint8_t > & _buffer;
        IVector< uint8_t > & _responseBuffer;

        Delegate< void() > _onUpdate;
    };
}    // namespace AsnPlus::Cloud

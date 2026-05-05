#pragma once

#include "program/config.hpp"

#include "cJSON.h"

#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/vector.hpp"

#include "firestore_request.hpp"

namespace AsnPlus::Cloud
{
    template< typename REQUEST, typename RESPONSE = REQUEST >
    class ObjectPostRequest : public IFirestoreRequest
    {
    public:
        ObjectPostRequest(
            REQUEST &                   req,
            RESPONSE &                  response,
            IFirestoreRequest::Config & config,
            IVector< uint8_t > &        buffer,
            IVector< uint8_t > &        responseBuffer,
            Delegate< void() >          onUpdate
        ) :
            IFirestoreRequest( config ),
            _config( config ),
            _request( req ),
            _response( response ),
            _buffer( buffer ),
            _responseBuffer( responseBuffer ),
            _onUpdate( onUpdate )
        {
        }

        bool initialize() override
        {
            if ( _buffer.capacity() == 0 ) return false;

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

            const size_t requestLen = strlen( reinterpret_cast< char * >( _buffer.data() ) );
            const auto * bufData    = reinterpret_cast< const uint8_t * >( _buffer.data() );
            _buffer.assign( bufData, bufData + requestLen );

            char requestLabel[ 64 ];
            snprintf( requestLabel, sizeof( requestLabel ), "Request [%s]", _config.moduleUrl );
            Log::hexdump( requestLabel, _buffer.data(), _buffer.size() );
            cJSON_Delete( root );

            _responseBuffer.clear();
            Https::Request  httpReq { .method = Https::Method::POST, .payload = &_buffer };
            Https::Response resp    { .response = &_responseBuffer };

            uint16_t status = request( &httpReq, &resp );

            if ( status != 200 )
            {
                Log::error( "Request (%s) failed with status code %d", _url, status );
                return false;
            }

            if ( _responseBuffer.empty() ) return true;

            char responseLabel[ 64 ];
            snprintf( responseLabel, sizeof( responseLabel ), "Response [%s]", _config.moduleUrl );
            Log::hexdump( responseLabel, _responseBuffer.data(), _responseBuffer.size() );

            cJSON * responseJson = cJSON_Parse( reinterpret_cast< char * >( _responseBuffer.data() ) );
            if ( ! responseJson )
            {
                Log::error( "JSON parsing failed (%s)", _url );
                return false;
            }

            const uint64_t oldTimestamp = _response.timestamp;
            _response.fromJson( responseJson );
            if ( _response.timestamp != oldTimestamp )
            {
                _onUpdate();
            }

            cJSON_Delete( responseJson );
            return true;
        }

    private:
        static constexpr const char TAG[] = "ObjectPostRequest";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_CLOUD_REQUESTS, TAG >;

        IFirestoreRequest::Config & _config;

        REQUEST &            _request;
        RESPONSE &           _response;
        IVector< uint8_t > & _buffer;
        IVector< uint8_t > & _responseBuffer;

        Delegate< void() > _onUpdate;
    };
}    // namespace AsnPlus::Cloud

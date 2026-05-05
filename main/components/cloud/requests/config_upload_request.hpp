#pragma once

#include "program/config.hpp"

#include "cJSON.h"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/vector.hpp"

#include "firestore_request.hpp"

namespace AsnPlus::Cloud
{
    template< typename T, bool WITH_CHANNEL = false >
    class ConfigUploadRequest : public IFirestoreRequest
    {
    public:
        ConfigUploadRequest(
            T &                         data,
            IFirestoreRequest::Config & config,
            IVector< uint8_t > &        buffer,
            IVector< uint8_t > &        responseBuffer
        ) :
            IFirestoreRequest( config ),
            _config( config ),
            _data( data ),
            _buffer( buffer ),
            _responseBuffer( responseBuffer )
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

            if constexpr ( WITH_CHANNEL )
            {
                char channelStr[ 4 ];
                snprintf( channelStr, sizeof( channelStr ), "%u", static_cast< unsigned >( _config.objectId ) );
                strncat( _url, "&channel=", sizeof( _url ) - strlen( _url ) - 1 );
                strncat( _url, channelStr, sizeof( _url ) - strlen( _url ) - 1 );
            }

            return true;
        }

        bool send() override
        {
            cJSON * json = cJSON_CreateObject();
            if ( ! json )
            {
                Log::error( "Failed to allocate JSON (%s)", _url );
                return false;
            }

            toJson( _data, json );

            char * body = cJSON_PrintUnformatted( json );
            cJSON_Delete( json );

            if ( ! body )
            {
                Log::error( "Failed to serialize JSON (%s)", _url );
                return false;
            }

            const size_t bodyLen  = strlen( body );
            const size_t copyLen  = bodyLen < _buffer.capacity() ? bodyLen : _buffer.capacity();
            const auto * bodyData = reinterpret_cast< const uint8_t * >( body );
            _buffer.assign( bodyData, bodyData + copyLen );
            cJSON_free( body );

            _responseBuffer.clear();
            Https::Request  req  { .method = Https::Method::POST, .payload = &_buffer };
            Https::Response resp { .response = &_responseBuffer };

            uint16_t status = request( &req, &resp );

            if ( status != 200 )
            {
                Log::error( "Request (%s) failed with status code %d", _url, status );
                return false;
            }

            Log::info( "Request (%s) succeeded with status code %d", _url, status );
            return true;
        }

    private:
        static constexpr const char TAG[] = "ConfigUploadRequest";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_CLOUD_REQUESTS, TAG >;

        IFirestoreRequest::Config & _config;
        T &                         _data;
        IVector< uint8_t > &        _buffer;
        IVector< uint8_t > &        _responseBuffer;
    };
}    // namespace AsnPlus::Cloud

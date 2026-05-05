#pragma once

#include "program/config.hpp"

#include "cJSON.h"

#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/vector.hpp"

#include "firestore_request.hpp"

#include "components/cloud/json_conversion/time_config.hpp"

namespace AsnPlus::Cloud
{
    class TimeConfigRequest : public IFirestoreRequest
    {
    public:
        TimeConfigRequest(
            TimeConfig &                response,
            IFirestoreRequest::Config & config,
            IVector< uint8_t > &        responseBuffer,
            Delegate< void() >          onUpdate
        ) :
            IFirestoreRequest( config ),
            _config( config ),
            _response( response ),
            _responseBuffer( responseBuffer ),
            _onUpdate( onUpdate )
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
            _responseBuffer.clear();
            Https::Request  req  { .method = Https::Method::GET };
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

            const uint64_t oldTimestamp = _response.timestamp;
            fromJson( _response, responseJson );
            if ( _response.timestamp != oldTimestamp )
            {
                _onUpdate();
            }

            cJSON_Delete( responseJson );
            return true;
        }

    private:
        static constexpr const char TAG[] = "TimeConfigRequest";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_CLOUD_REQUESTS, TAG >;

        IFirestoreRequest::Config & _config;

        TimeConfig &         _response;
        IVector< uint8_t > & _responseBuffer;

        Delegate< void() > _onUpdate;
    };

    class TimeConfigPostRequest : public IFirestoreRequest
    {
    public:
        TimeConfigPostRequest(
            AsnPlus::TimeConfig &       data,
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
        static constexpr const char TAG[] = "TimeConfigPostRequest";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_CLOUD_REQUESTS, TAG >;

        IFirestoreRequest::Config & _config;
        AsnPlus::TimeConfig &       _data;
        IVector< uint8_t > &        _buffer;
        IVector< uint8_t > &        _responseBuffer;
    };
}    // namespace AsnPlus::Cloud

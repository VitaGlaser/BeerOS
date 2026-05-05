#pragma once

#include "program/config.hpp"

#include "cJSON.h"

#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/vector.hpp"

#include "firestore_request.hpp"

#include "components/cloud/json_conversion/channel_config.hpp"

namespace AsnPlus::Cloud
{
    class ChannelConfigRequest : public IFirestoreRequest
    {
    public:
        ChannelConfigRequest(
            ChannelConfig &             response,
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

            char channelStr[ 4 ];
            snprintf( channelStr, sizeof( channelStr ), "%u", static_cast< unsigned >( _config.objectId ) );
            strncat( _url, "&channel=", sizeof( _url ) - strlen( _url ) - 1 );
            strncat( _url, channelStr, sizeof( _url ) - strlen( _url ) - 1 );

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
        static constexpr const char TAG[] = "ChannelConfigRequest";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_CLOUD_REQUESTS, TAG >;

        IFirestoreRequest::Config & _config;

        ChannelConfig &      _response;
        IVector< uint8_t > & _responseBuffer;

        Delegate< void() > _onUpdate;
    };
}    // namespace AsnPlus::Cloud

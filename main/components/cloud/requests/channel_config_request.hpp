#pragma once

#include "program/config.hpp"

#include "cJSON.h"

#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/vector.hpp"

#include "asn/asn-esp32-wifi/https/client/ifirestore_request.hpp"

#include "asn/asn-hal/common/common_structs.hpp"

#include "components/cloud/json_conversion/channel_config.hpp"


namespace AsnPlus::Cloud
{
    class ChannelConfigRequest : public Esp32::Https::IFirestoreRequest
    {
    public:
        ChannelConfigRequest(
            ChannelConfig &                           response,
            Esp32::Https::IFirestoreRequest::Config & config,
            IVector< uint8_t > &                      responseBuffer,
            Delegate< void() >                        onUpdate
        ) :
            IFirestoreRequest( config ),
            _config( config ),
            _response( response ),
            _responseBuffer( responseBuffer ),
            _onUpdate( onUpdate )
        {
            _config.method = Esp32::Https::IClient::Method::GET;
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

            char channel_str[ 4 ];
            snprintf( channel_str, sizeof( channel_str ), "%u", static_cast< unsigned >( _config.objectId ) );
            strncat( _url, "&channel=", sizeof( _url ) - strlen( _url ) - 1 );
            strncat( _url, channel_str, sizeof( _url ) - strlen( _url ) - 1 );

            return true;
        }

        bool send() override
        {
            _responseBuffer.resize( _responseBuffer.capacity() );
            uint32_t responseLen = static_cast< uint32_t >( _responseBuffer.size() );

            uint32_t ret         = request( nullptr, 0, nullptr, 0, _responseBuffer.data(), responseLen );

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
            fromJson( _response, responseJson );
            if ( _response.timestamp != old_timestamp )
            {
                _onUpdate();
            }

            cJSON_Delete( responseJson );
            return true;
        }

    private:
        static constexpr const char TAG[] = "ChannelConfigRequest";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_CLOUD_REQUESTS, TAG >;

        Esp32::Https::IFirestoreRequest::Config & _config;

        ChannelConfig &      _response;
        IVector< uint8_t > & _responseBuffer;

        Delegate< void() > _onUpdate;
    };
}    // namespace AsnPlus::Cloud

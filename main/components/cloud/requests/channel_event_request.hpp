#pragma once

#include "program/config.hpp"

#include "cJSON.h"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/vector.hpp"

#include "asn/asn-esp32-wifi/https/client/ifirestore_request.hpp"

#include "asn/asn-hal/common/common_structs.hpp"

#include "components/cloud/json_conversion/channel_event.hpp"

namespace AsnPlus::Cloud
{
    class ChannelEventRequest : public Esp32::Https::IFirestoreRequest
    {
    public:
        ChannelEventRequest(
            IRingBuffer< EventMonitor::Event > &      eventHistory,
            Esp32::Https::IFirestoreRequest::Config & config,
            IVector< uint8_t > &                      responseBuffer
        ) :
            IFirestoreRequest( config ),
            _config( config ),
            _eventHistory( eventHistory ),
            _responseBuffer( responseBuffer )
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

            char channel_str[ 4 ];
            snprintf( channel_str, sizeof( channel_str ), "%u", static_cast< unsigned >( _config.objectId ) );
            strncat( _url, "&channel=", sizeof( _url ) - strlen( _url ) - 1 );
            strncat( _url, channel_str, sizeof( _url ) - strlen( _url ) - 1 );

            return true;
        }

        bool send() override
        {
            for ( auto & event : _eventHistory )
            {
                if ( event.startTimestamp == 0 || event.endTimestamp == 0 )
                    continue;

                if ( event.synced )
                    continue;

                bool sent = _sendEvent( event );
                if ( sent )
                {
                    event.synced = true;
                }
                else
                {
                    Log::warn( "Failed to send event seq=%llu — will retry next poll", event.sequenceNumber );
                }

                return sent;
            }

            return true;
        }

    private:
        static constexpr const char TAG[] = "ChannelEventRequest";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_CLOUD_REQUESTS, TAG >;

        Esp32::Https::IFirestoreRequest::Config & _config;

        IRingBuffer< EventMonitor::Event > & _eventHistory;
        IVector< uint8_t > &                 _responseBuffer;

        bool _sendEvent( EventMonitor::Event & event )
        {
            cJSON * requestJson = cJSON_CreateObject();
            if ( ! requestJson )
            {
                Log::error( "Failed to allocate request JSON" );
                return false;
            }

            cJSON * eventJson = cJSON_CreateObject();
            if ( ! eventJson )
            {
                Log::error( "Failed to allocate event JSON" );
                cJSON_Delete( requestJson );
                return false;
            }

            toJson( event, eventJson );
            cJSON_AddItemToObject( requestJson, ChannelEventRequestJson::EVENT_TAG, eventJson );

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

            return true;
        }
    };
}    // namespace AsnPlus::Cloud

#pragma once

#include "program/config.hpp"

#include "cJSON.h"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/vector.hpp"

#include "firestore_request.hpp"

#include "components/cloud/json_conversion/channel_event.hpp"

namespace AsnPlus::Cloud
{
    class ChannelEventRequest : public IFirestoreRequest
    {
    public:
        ChannelEventRequest(
            IRingBuffer< EventMonitor::Event > & eventHistory,
            IFirestoreRequest::Config &          config,
            IVector< uint8_t > &                 requestBuffer,
            IVector< uint8_t > &                 responseBuffer
        ) :
            IFirestoreRequest( config ),
            _config( config ),
            _eventHistory( eventHistory ),
            _requestBuffer( requestBuffer ),
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

            char channelStr[ 4 ];
            snprintf( channelStr, sizeof( channelStr ), "%u", static_cast< unsigned >( _config.objectId ) );
            strncat( _url, "&channel=", sizeof( _url ) - strlen( _url ) - 1 );
            strncat( _url, channelStr, sizeof( _url ) - strlen( _url ) - 1 );

            return true;
        }

        bool send() override
        {
            for ( auto & event : _eventHistory )
            {
                if ( event.startTimestamp == 0 || event.endTimestamp == 0 ) continue;

                if ( event.synced ) continue;

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

        IFirestoreRequest::Config &          _config;
        IRingBuffer< EventMonitor::Event > & _eventHistory;
        IVector< uint8_t > &                 _requestBuffer;
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

            Log::info( "Request (%s) succeeded with status code %d", _url, status );
            return true;
        }
    };
}    // namespace AsnPlus::Cloud

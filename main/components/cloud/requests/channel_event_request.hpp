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
            IVector< uint8_t > &                 responseBuffer,
            bool                                 includeProfiles = true,
            bool                                 useSyncedFlag   = true,
            bool                                 useDirectUrl    = false
        ) :
            IFirestoreRequest( config ),
            _config( config ),
            _eventHistory( eventHistory ),
            _requestBuffer( requestBuffer ),
            _responseBuffer( responseBuffer ),
            _includeProfiles( includeProfiles ),
            _useSyncedFlag( useSyncedFlag ),
            _useDirectUrl( useDirectUrl )
        {
        }

        bool initialize() override
        {
            if ( _useDirectUrl )
            {
                snprintf( _url, sizeof( _url ), "%s", _config.baseUrl );
            }
            else
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
            }

            char channelStr[ 4 ];
            snprintf( channelStr, sizeof( channelStr ), "%u", static_cast< unsigned >( _config.objectId ) );
            strncat( _url, strchr( _url, '?' ) ? "&channel=" : "?channel=", sizeof( _url ) - strlen( _url ) - 1 );
            strncat( _url, channelStr, sizeof( _url ) - strlen( _url ) - 1 );

            if ( ! _useSyncedFlag && ! _eventHistory.empty() )
            {
                _lastSentSequenceNumber = _eventHistory.back().sequenceNumber;
            }

            return true;
        }

        bool send() override
        {
            for ( auto & event : _eventHistory )
            {
                if ( event.startTimestamp == 0 || event.endTimestamp == 0 ) continue;

                if ( _useSyncedFlag )
                {
                    if ( event.synced ) continue;
                }
                else
                {
                    if ( event.sequenceNumber <= _lastSentSequenceNumber ) continue;
                }

                bool sent = _sendEvent( event );
                if ( sent )
                {
                    if ( _useSyncedFlag ) event.synced = true;
                    _lastSentSequenceNumber = event.sequenceNumber;
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
        bool                                 _includeProfiles       = true;
        bool                                 _useSyncedFlag         = true;
        bool                                 _useDirectUrl          = false;
        uint64_t                             _lastSentSequenceNumber = 0;

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

            toJson( event, eventJson, _includeProfiles );
            cJSON_AddItemToObject( requestJson, ChannelEventRequestJson::EVENT_TAG, eventJson );

            char * body = cJSON_PrintUnformatted( requestJson );
            cJSON_Delete( requestJson );

            if ( ! body )
            {
                Log::error( "Failed to serialize request JSON" );
                return false;
            }

            const size_t bodyLen  = strlen( body );
            if ( bodyLen > _requestBuffer.capacity() )
            {
                Log::error(
                    "Request body too large (%u > %u), dropping event seq=%llu",
                    static_cast< unsigned >( bodyLen ),
                    static_cast< unsigned >( _requestBuffer.capacity() ),
                    event.sequenceNumber
                );
                cJSON_free( body );
                return false;
            }

            const size_t copyLen  = bodyLen;
            const auto * bodyData = reinterpret_cast< const uint8_t * >( body );
            _requestBuffer.assign( bodyData, bodyData + copyLen );
            cJSON_free( body );

            Log::warn(
                "EVT cloud_send url=%s seq=%llu startTs=%llu endTs=%llu volumeMl=%u pulses=%llu type=%u classification=%u payloadBytes=%u",
                _url,
                event.sequenceNumber,
                event.startTimestamp,
                event.endTimestamp,
                event.volume,
                event.pulseCount,
                static_cast< uint8_t >( event.type ),
                event.classification,
                static_cast< unsigned >( copyLen )
            );

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

#pragma once

#include <cstdio>
#include <cstring>

#include "cJSON.h"

#include "program/config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/vector.hpp"

#include "asn/asn-hal/include/common/identification_structs.hpp"
#include "asn/asn-hal/include/https/request.hpp"

namespace AsnPlus
{
    class IFirestoreRequest : public Https::IRequest
    {
    public:
        struct Config : Https::IRequest::Config
        {
            const char * uid       = nullptr;
            const char * moduleUrl = nullptr;
            uint8_t      objectId  = 0;
        };

        explicit IFirestoreRequest( Config & config ) : IRequest( config ) {}

    protected:
        Vector< Https::HeaderKeyValue, 4 > _headers {};

        void addBasicHeaders()
        {
            _headers.clear();
            _headers.push_back( { "Content-Type", "application/json" } );
        }

        static void formatUrl(
            char *       buffer,
            int          bufferSize,
            const char * baseUrl,
            const char * moduleUrl,
            const char * uid,
            uint16_t     uidLength,
            const char * /* password */
        )
        {
            char     uidStr[ ManufactureInfo::UID_LENGTH + 1 ] = {};
            uint16_t copyLen = uidLength < ManufactureInfo::UID_LENGTH ? uidLength : ManufactureInfo::UID_LENGTH;
            memcpy( uidStr, uid, copyLen );
            snprintf( buffer, static_cast< size_t >( bufferSize ), "%s/%s?uid=%s", baseUrl, moduleUrl, uidStr );
        }

        uint16_t request( Https::Request * request, Https::Response * response )
        {
            if ( request->method == Https::Method::POST )
            {
                addBasicHeaders();
                request->headers = &_headers;
            }

            return IRequest::request( request, response );
        }

    private:
        static constexpr const char TAG[] = "IFirestoreRequest";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_CLOUD_REQUESTS, TAG >;
    };
}    // namespace AsnPlus

#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/etl/to_arithmetic.h"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/string.hpp"
#include "asn/asn-core/string_util.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/vector.hpp"

#include "asn/asn-hal/include/https/client.hpp"

#include "../eg915.hpp"

namespace AsnPlus
{
    /**
     * @brief HTTPS client using EG915 raw SSL sockets with manually constructed HTTP/1.1 requests.
     *
     * Opens a TLS connection via AT+QSSLOPEN, sends a hand-built HTTP/1.1 request via
     * AT+QSSLSEND, reads the raw response via AT+QSSLRECV, and parses the HTTP status
     * line and body from the received bytes.
     */
    class Eg915HttpsClient : public Https::IClient
    {
    public:
        explicit Eg915HttpsClient( Eg915 & modem );

        bool initialize() override;
        bool setHeaders( const IVector< Https::HeaderKeyValue > & headers ) override;
        void cleanup() override;

    protected:
        uint16_t get( const char * url, Https::Response * response ) override;
        uint16_t post( const char * url, Https::Request * request, Https::Response * response ) override;

    private:
        static constexpr const char TAG[]            = "Eg915HttpsClient";
        using Log                                    = Logger< AsnPlus::ModuleConfig::Eg915::LOG_LEVEL, TAG >;

        static constexpr uint32_t RECV_TIMEOUT_MS    = 30'000;
        static constexpr uint16_t SSL_PORT           = 443;
        static constexpr size_t   MAX_REQUEST_SIZE   = 1024;
        static constexpr size_t   MAX_HOST_LENGTH    = 128;
        static constexpr size_t   MAX_QSSLRECV_BYTES = 1460;
        static constexpr size_t   MAX_SSL_READ_CHUNK =
            ( MAX_QSSLRECV_BYTES < AsnPlus::ModuleConfig::Eg915::DATA_BUFFER_SIZE - 4 )
            ? MAX_QSSLRECV_BYTES
            : AsnPlus::ModuleConfig::Eg915::DATA_BUFFER_SIZE - 4;

        Eg915 & _modem;

        const IVector< Https::HeaderKeyValue > * _customHeaders = nullptr;

        // MARK: URL

        struct UrlParts
        {
            StringView host;
            StringView path;
            uint16_t   port = SSL_PORT;
        };

        static bool _parseUrl( const char * url, UrlParts & parts );

        // MARK: Request

        uint16_t _doRequest( const char * method, const char * url, Https::Request * req, Https::Response * response );

        void _buildRequestHeaders(
            String< MAX_REQUEST_SIZE > &             request,
            const char *                             method,
            const UrlParts &                         parts,
            const IVector< Https::HeaderKeyValue > * request_headers,
            uint32_t                                 content_length
        );

        // MARK: Response

        uint16_t        _receiveResponse( Https::Response * response );
        void            _drainSslBuffer( IVector< uint8_t > & buf );
        static uint16_t _parseHttpResponse( const IVector< uint8_t > & data, Https::Response * response );
    };
}    // namespace AsnPlus

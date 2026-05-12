#pragma once

#include "asn_module_config.hpp"

#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "psa/crypto.h"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/vector.hpp"

#include "asn/asn-hal/include/https/client.hpp"

namespace AsnPlus::Esp32::Https
{
    class Client : public AsnPlus::Https::IClient
    {
    public:
        bool initialize() override;
        bool setHeaders( const IVector< AsnPlus::Https::HeaderKeyValue > & headers ) override;
        void cleanup() override;

    protected:
        uint16_t get( const char * url, AsnPlus::Https::Response * response ) override;
        uint16_t
            post( const char * url, AsnPlus::Https::Request * request, AsnPlus::Https::Response * response ) override;

    private:
        static constexpr const char TAG[] = "Https::Client";
        using Log                         = Logger< ModuleConfig::Https::LOG_LEVEL, TAG >;

        esp_http_client_config_t                                   _httpConfig {};
        mbedtls_ssl_config                                         _tlsConfig {};
        esp_http_client_handle_t                                   _client {};
        Vector< uint8_t, ModuleConfig::Https::CLIENT_BUFFER_SIZE > _responseBuffer {};

        void _reinit();
        bool _setUrl( const char * url );

        static bool      _appendTrunc( IVector< uint8_t > & vector, const uint8_t * data, size_t length );
        static esp_err_t _httpEventHandler( esp_http_client_event_t * evt );
    };
}    // namespace AsnPlus::Esp32::Https

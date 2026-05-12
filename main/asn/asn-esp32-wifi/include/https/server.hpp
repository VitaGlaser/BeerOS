#pragma once

#include "asn_module_config.hpp"

#include "esp_https_server.h"
#include "esp_netif.h"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/vector.hpp"

#include "asn/asn-hal/include/https/server.hpp"

namespace AsnPlus::Esp32::Https
{
    class Server : public AsnPlus::Https::IServer
    {
    public:
        struct Config : public AsnPlus::Https::IServer::Config
        {
            uint32_t taskAffinity = 0;
        };

        Server( Config & config );

        bool initialize() override;
        bool deinitialize() override;
        bool start();
        bool stop();
        bool addRequestHandler( AsnPlus::Https::IHandler * handler ) override;

    protected:
        static constexpr uint32_t TASK_STACK_SIZE = 8192;
        Config &                  _config;

        httpd_handle_t _server = nullptr;

        httpd_uri_t _root {
            .uri      = "/",
            .method   = HTTP_GET,
            .handler  = _rootGetHandler,
            .user_ctx = nullptr,
        };

    private:
        static constexpr const char TAG[]     = "Server";
        using Log                             = Logger< ModuleConfig::Https::LOG_LEVEL, TAG >;

        static constexpr uint8_t  MAX_HEADERS = 16;
        static constexpr uint16_t BUFFER_SIZE = 4096;

        uint8_t * _buffer;

        Vector< httpd_uri_t, ModuleConfig::Https::MAX_HANDLERS > _nativeHandlers {};

        static const char * statusToStr( uint16_t code );
        httpd_method_t      _getMethod( AsnPlus::Https::Method method );
        static bool _getHeadersFromRequest( httpd_req_t * req, IVector< AsnPlus::Https::HeaderKeyValue > & headers );
        static bool _fillBufferFromRequest( httpd_req_t * req, uint8_t * buffer, size_t content_len );
        static esp_err_t _generalHandler( httpd_req_t * req );
        static esp_err_t _rootGetHandler( httpd_req_t * req );
        static void      _printPeerCertInfo( const mbedtls_ssl_context * ssl );
        static void      _serverCallback( esp_https_server_user_cb_arg_t * user_cb );
    };
}    // namespace AsnPlus::Esp32::Https

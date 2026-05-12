#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/array.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/string.hpp"
#include "asn/asn-core/types.hpp"

#include "request.hpp"

#include "structs.hpp"

namespace AsnPlus::Https
{
    class IHandler;

    class IServer
    {
    public:
        struct Config
        {
            bool      enabled           = true;
            uint16_t  port              = 443;
            uint8_t   maxConnections    = 4;
            IString * serverCertPem     = nullptr;
            IString * privateKeyPem     = nullptr;
            bool      requireClientCert = false;
            bool      sslEnabled        = false;
        };

        IServer( Config & config ) : _config( config ) {}

        virtual bool initialize()                            = 0;
        virtual bool deinitialize()                          = 0;
        virtual bool start()                                 = 0;
        virtual bool stop()                                  = 0;
        virtual bool addRequestHandler( IHandler * handler ) = 0;

    protected:
        Array< IHandler *, ModuleConfig::Https::MAX_HANDLERS > _handlers;

        Config & _config;

    private:
        static constexpr const char TAG[] = "IServer";
    };

    class IHandler
    {
    public:
        using Path = String< MAX_URL_LENGTH >;

        struct Config
        {
            Method method;
            Path   path;
        };

        IHandler( IServer & server, Config & config ) : _server( server ), _config( config )
        {
            _server.addRequestHandler( this );
        }

        const Path & getPath() const { return _config.path; }

        const Method & getMethod() const { return _config.method; }

        uint8_t * getBuffer() { return _buffer; }

        void setBuffer( uint8_t * buffer ) { _buffer = buffer; }

        virtual bool handle( Request & request, Response & response ) = 0;

    private:
        IServer & _server;
        Config &  _config;
        uint8_t * _buffer;
    };
}    // namespace AsnPlus::Https

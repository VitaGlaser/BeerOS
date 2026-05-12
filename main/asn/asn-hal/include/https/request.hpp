#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "client.hpp"
#include "structs.hpp"

namespace AsnPlus::Https
{
    class IRequest
    {
    public:
        struct Config
        {
            const char * baseUrl;
        };

        IRequest( Config & config );

        virtual bool initialize() = 0;
        virtual bool send()       = 0;

        void setClient( IClient * client );

    protected:
        IClient * _client = nullptr;
        Config &  _config;
        char      _url[ MAX_URL_LENGTH ] = { 0 };

        uint16_t request( Request * request, Response * response );

    private:
        static constexpr const char TAG[] = "IRequest";
        using Log                         = Logger< ModuleConfig::Https::LOG_LEVEL, TAG >;
    };
}    // namespace AsnPlus::Https

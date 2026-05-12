#pragma once

#include "asn/asn-core/types.hpp"

namespace AsnPlus
{
    class IUart
    {
    public:
        struct Config
        {
            uint32_t baudRate;
        };

        IUart( Config & config ) : _config( config ) {}

        virtual bool    initialize()                                      = 0;
        virtual int32_t write( const char * data, size_t len )            = 0;
        virtual int32_t read( char * data, size_t len, uint32_t timeout ) = 0;

    protected:
        static constexpr const char TAG[] = "IUart";

        Config & _config;
    };
}    // namespace AsnPlus
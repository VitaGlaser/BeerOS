#pragma once

#include "asn/asn-core/types.hpp"

namespace AsnPlus
{
    class IWatchdog
    {
    public:
        struct Config
        {
            uint32_t timeoutMs = 5000;
        };

        IWatchdog( Config & config ) : _config( config ) {}

        virtual bool initialize() = 0;

        virtual void feed()       = 0;

        virtual void enable()     = 0;
        virtual void disable()    = 0;

    protected:
        Config & _config;
    };
}    // namespace AsnPlus

#pragma once

#include "asn/asn-core/types.hpp"

namespace AsnPlus
{
    class IAdcOneshot
    {
    public:
        class IChannel
        {
        public:
            struct Config
            {
                uint8_t channel;
            };

            IChannel( Config & config ) : _config( config ) {}

            virtual bool initialize()  = 0;

            virtual Config getConfig() = 0;

        protected:
            static constexpr const char TAG[] = "IChannel";
            Config &                    _config;
        };

        struct Config
        {
            uint8_t unit;
        };

        IAdcOneshot( Config & config ) : _config( config ) {}

        virtual bool initialize()                          = 0;
        virtual bool addChannel( IChannel & channel )      = 0;

        virtual uint32_t readRaw( IChannel & channel )     = 0;
        virtual uint32_t readVoltage( IChannel & channel ) = 0;

    protected:
        static constexpr const char TAG[] = "IAdcOneshot";

        Config & _config;
    };
}    // namespace AsnPlus

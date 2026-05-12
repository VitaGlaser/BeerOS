#pragma once

#include "asn/asn-core/types.hpp"
#include "asn/asn-core/vector.hpp"

namespace AsnPlus
{
    class IPwm
    {
    public:
        class IChannel
        {
        public:
            struct Config
            {
                uint8_t channel;             // peripheral channel index
                bool    inverted = false;    // per-channel polarity if supported
            };

            IChannel( Config & config, IPwm & unit ) : _config( config ), _unit( unit ) { _unit.addChannel( *this ); }

            virtual bool initialize()                     = 0;

            // Duty cycle: 0.0...100.0 %
            virtual void  setDutyCycle( float dutyCycle ) = 0;
            virtual float getDutyCycle()                  = 0;

            virtual Config getConfig()                    = 0;

        protected:
            static constexpr const char TAG[] = "IPwm::IChannel";
            Config &                    _config;
            IPwm &                      _unit;
        };

        struct Config
        {
            uint8_t  unit;
            uint32_t frequencyHz;
        };

        IPwm( Config & config ) : _config( config ) {}

        virtual bool initialize() = 0;
        virtual void poll()       = 0;

        virtual bool addChannel( IChannel & channel )
        {
            if ( _channels.size() >= MAX_CHANNELS ) return false;
            _channels.push_back( &channel );
            return true;
        }

        virtual bool start()                                    = 0;
        virtual bool stop()                                     = 0;

        virtual void     setFrequencyHz( uint32_t frequencyHz ) = 0;
        virtual uint32_t getFrequencyHz()                       = 0;

    protected:
        static constexpr const char TAG[]        = "IPwm";
        static constexpr uint8_t    MAX_CHANNELS = 16;

        Config &                                 _config;
        Vector< IChannel *, MAX_CHANNELS > _channels {};
    };
}    // namespace AsnPlus
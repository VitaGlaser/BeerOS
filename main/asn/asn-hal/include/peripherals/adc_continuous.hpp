#pragma once

#include "asn/asn-core/ring_buffer.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/vector.hpp"

namespace AsnPlus
{
    class IAdcContinuous
    {
    public:
        class IChannel
        {
        public:
            struct Config
            {
                uint8_t channel;
                bool    useMilliVolts = false;
            };

            IChannel( Config & config, IRingBuffer< uint32_t > & buffer, IAdcContinuous & adc ) :
                _config( config ),
                _buffer( buffer ),
                _adc( adc )
            {
                _adc.addChannel( *this );
            }

            virtual bool initialize() = 0;

            void addSample( uint32_t sample ) { _buffer.push( sample ); }

            virtual Config getConfig() = 0;

            IRingBuffer< uint32_t > & getBuffer() { return _buffer; }

        protected:
            static constexpr const char TAG[] = "IChannel";

            Config &                  _config;
            IRingBuffer< uint32_t > & _buffer;
            IAdcContinuous &          _adc;
        };

        struct Config
        {
            uint8_t  unit;
            uint32_t samplingFrequencyHz;
        };

        IAdcContinuous( Config & config ) : _config( config ) {}

        virtual bool initialize() = 0;
        virtual void poll()       = 0;

        virtual bool addChannel( IChannel & channel )
        {
            if ( _channels.size() >= MAX_CHANNELS ) return false;
            _channels.push_back( &channel );
            return true;
        }

        virtual bool start() = 0;
        virtual bool stop()  = 0;

    protected:
        static constexpr const char TAG[]        = "IAdcContinuous";
        static constexpr uint8_t    MAX_CHANNELS = 16;

        Config &                           _config;
        Vector< IChannel *, MAX_CHANNELS > _channels {};
    };
}    // namespace AsnPlus

#pragma once

#include "program/config.hpp"

#include "asn/asn-core/logger.hpp"

#include "asn/asn-hal/include/time_manager/time_manager.hpp"

#include "asn/asn-expander-lib/include/timer/timer.hpp"

#include "base.hpp"

namespace AsnPlus::DataSource
{
    class FlowPulse : public Base
    {
    public:
        FlowPulse( uint16_t pulsesPerLitre ) : Base(), _pulsesPerLitre( pulsesPerLitre ) {}

        bool initialize() override
        {
            Log::info( "Initialized (pulses/L: %u)", _pulsesPerLitre );
            return true;
        }

        void poll() override
        {
            if ( _channel == nullptr )
            {
                _clearSample();
                return;
            }

            uint16_t count     = _channel->getValue();
            uint64_t timestamp = TimeManager::instance().getUtcTime().toEpochMillis();

            uint16_t deltaPulses =
                ( count >= _lastCount ) ? ( count - _lastCount ) : ( ( 0xFFFF - _lastCount ) + count + 1 );

            uint32_t value = 0;
            if ( _lastTimestamp > 0 )
            {
                uint64_t deltaTimeMs = timestamp - _lastTimestamp;
                if ( deltaTimeMs > 0 )
                {
                    value = static_cast< uint32_t >(
                        static_cast< float >( deltaPulses * ML_PER_LITRE * MILLIS_PER_MIN ) /
                        ( static_cast< float >( _pulsesPerLitre * deltaTimeMs ) )
                    );
                }
            }

            _lastCount     = count;
            _lastTimestamp = timestamp;

            // Store pulse count in debug info for visibility;
            _debugInfo.data[ 0 ] = static_cast< uint8_t >( ( count >> 8 ) & 0xFF );
            _debugInfo.data[ 1 ] = static_cast< uint8_t >( count & 0xFF );

            _writeSample( timestamp, value );

            Log::debug( "Polled pulse flow sensor: %u ml/min (%llu ms)", value, timestamp );
        }

        void bindTimerChannel( Expander::Timers::Timer::Channel & channel ) { _channel = &channel; }

        void unbindTimerChannel() { _channel = nullptr; }

    private:
        static constexpr const char TAG[]           = "FlowPulseDataSource";
        using Log                                   = Logger< ProjectConfig::LOG_LEVEL_DATA_SOURCES, TAG >;

        static constexpr float ML_PER_LITRE         = 1000.0f;

        Expander::Timers::Timer::Channel * _channel = nullptr;
        uint16_t                           _pulsesPerLitre;

        uint16_t _lastCount     = 0;
        uint64_t _lastTimestamp = 0;
    };
}    // namespace AsnPlus::DataSource

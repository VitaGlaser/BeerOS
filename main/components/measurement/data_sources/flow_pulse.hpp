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

            const float flowRate =
                _channel->getFrequency() / static_cast< float >( _pulsesPerLitre ) * TO_ML_PER_MINUTE;

            uint64_t ts  = TimeManager::instance().getRuntime().utcEpochMs;
            uint32_t val = static_cast< uint32_t >( flowRate );
            _writeSample( ts, val );

            Log::debug( "Polled pulse flow sensor: %u ml/min (%llu ms)", val, ts );
        }

        void bindTimerChannel( Expander::Timers::Timer::Channel & channel ) { _channel = &channel; }

        void unbindTimerChannel() { _channel = nullptr; }

    private:
        static constexpr const char TAG[]           = "FlowPulseDataSource";
        using Log                                   = Logger< ProjectConfig::LOG_LEVEL_DATA_SOURCES, TAG >;

        static constexpr float TO_ML_PER_MINUTE     = 60 * 1000.0f;

        Expander::Timers::Timer::Channel * _channel = nullptr;
        uint16_t                           _pulsesPerLitre;
    };
}    // namespace AsnPlus::DataSource

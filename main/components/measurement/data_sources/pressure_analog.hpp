#pragma once

#include "program/config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "asn/asn-expander-lib/include/adc/adc.hpp"
#include "asn/asn-hal/include/time_manager/time_manager.hpp"

#include "base.hpp"

namespace AsnPlus::DataSource
{
    class PressureAnalog : public Base
    {
    public:
        PressureAnalog() : Base() {}

        bool initialize() override
        {
            Log::debug( "Initializing" );

            Log::info( "Initialized" );
            return true;
        }

        void poll() override
        {
            if ( ! _enabled ) return;

            if ( _channel == nullptr )
            {
                _clearSample();
                return;
            }

            const uint32_t rawValue = static_cast< uint32_t >( _channel->getValue() );
            const uint64_t ts       = TimeManager::instance().getRuntime().utcEpochMs;
            _writeSample( ts, rawValue );

            Log::debug( "Polled analog pressure sensor (ID: %llu): %u (%llu ms)", _id, rawValue, ts );
        }

        void bindAdcChannel( Expander::Adc::Adc::Channel & channel ) { _channel = &channel; }
        void unbindAdcChannel() { _channel = nullptr; }

    private:
        static constexpr const char TAG[] = "PressureAnalogDataSource";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_DATA_SOURCES, TAG >;

        Expander::Adc::Adc::Channel * _channel = nullptr;
    };
}    // namespace AsnPlus::DataSource

#pragma once

#include "program/config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "asn/asn-hal/include/time_manager/time_manager.hpp"

#include "base.hpp"

namespace AsnPlus::DataSource
{
    class TemperatureAnalog : public Base
    {
    public:
        TemperatureAnalog() : Base() {}

        bool initialize() override
        {
            Log::debug( "Initializing" );

            Log::info( "Initialized" );
            return true;
        }

        void poll() override
        {
            if ( ! _enabled ) return;

            // TODO: read ADC channel and convert to temperature
            Log::debug( "Polled analog temperature sensor (ID: %llu)", _id );
        }

    private:
        static constexpr const char TAG[] = "TemperatureAnalogDataSource";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_DATA_SOURCES, TAG >;
    };
}    // namespace AsnPlus::DataSource

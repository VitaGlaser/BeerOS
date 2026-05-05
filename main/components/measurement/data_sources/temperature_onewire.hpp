#pragma once

#include "program/config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "asn/asn-hal/include/time_manager/time_manager.hpp"

#include "base.hpp"

namespace AsnPlus::DataSource
{
    class TemperatureOneWire : public Base
    {
    public:
        TemperatureOneWire() : Base() {}

        bool initialize() override
        {
            Log::debug( "Initializing" );

            Log::info( "Initialized" );
            return true;
        }

        void poll() override
        {
            if ( ! _enabled ) return;

            // TODO: issue 1-Wire convert + read scratchpad
            Log::debug( "Polled 1-Wire temperature sensor (ID: %llu)", _id );
        }

    private:
        static constexpr const char TAG[] = "TemperatureOneWireDataSource";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_DATA_SOURCES, TAG >;
    };
}    // namespace AsnPlus::DataSource

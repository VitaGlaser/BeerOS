#pragma once

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/ring_buffer.hpp"
#include "asn/asn-core/types.hpp"

#include "asn/asn-hal/include/common/common_structs.hpp"

#include "data_sources/base.hpp"

namespace AsnPlus
{
    class Sensor
    {
    public:
        struct Config : AsnPlus::Config
        {
            bool     enabled = false;
            uint64_t id      = 0;
        };

        Sensor( Config & config ) : _config( config ) {}

        bool initialize()
        {
            Log::debug( "Initializing" );

            Log::info( "Initialized" );
            return true;
        }

        void poll()
        {
            if ( ! _config.enabled )
            {
                Log::debug( "Sensor is disabled, skipping poll" );
                return;
            }

            if ( _dataSource == nullptr )
            {
                Log::error( "No data source configured for this sensor" );
                return;
            }

            _dataSource->poll();
        }

        DataSource::Base::Sample read() const
        {
            if ( _dataSource == nullptr )
            {
                Log::debug( "No data source configured for this sensor" );
                return {};
            }

            return _dataSource->read();
        }

        DataSource::Base::DebugInfo getDebugInfo() const
        {
            if ( _dataSource == nullptr )
            {
                Log::debug( "No data source configured for this sensor" );
                return {};
            }

            return _dataSource->getDebugInfo();
        }

        bool isDataSourceBound() const { return _dataSource != nullptr; }

        void bindDataSource( DataSource::Base & dataSource ) { _dataSource = &dataSource; }

        void unbindDataSource() { _dataSource = nullptr; }

    private:
        static constexpr const char TAG[] = "Sensor";
        using Log                         = AsnPlus::Logger< ProjectConfig::LOG_LEVEL_SENSOR, TAG >;

        Config & _config;

        DataSource::Base * _dataSource = nullptr;
    };
}    // namespace AsnPlus

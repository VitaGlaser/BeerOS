// TODO (DK): this should bind and unbind correctly the sources for each channel and its sensors as this is the class
// that know what index each channel has
#pragma once

#include "program/config.hpp"

#include "asn/asn-core/array.hpp"
#include "asn/asn-core/logger.hpp"

#include "channel.hpp"

#include "data_sources/manager.hpp"
#include "database/database.hpp"

namespace AsnPlus
{
    class ChannelManager
    {
    public:
        ChannelManager( DataSource::Manager & dataSourceManager, Database & database ) :
            _dataSourceManager( dataSourceManager ),
            _channels {
                Channel { database.channelConfigs[ 0 ], database.channelRuntimes[ 0 ], dataSourceManager, database.eventHistory0 },
                Channel { database.channelConfigs[ 1 ], database.channelRuntimes[ 1 ], dataSourceManager, database.eventHistory1 },
                Channel { database.channelConfigs[ 2 ], database.channelRuntimes[ 2 ], dataSourceManager, database.eventHistory2 },
                Channel { database.channelConfigs[ 3 ], database.channelRuntimes[ 3 ], dataSourceManager, database.eventHistory3 }
            }
        {
        }

        bool initialize()
        {
            Log::debug( "Initializing" );

            for ( auto & channel : _channels )
            {
                channel.initialize();
            }

            Log::info( "Initialized" );
            return true;
        }

        void poll()
        {

            uint8_t index = 0;
            for ( auto & channel : _channels )
            {
                _checkBinding( channel, index );
                channel.poll();
                ++index;
            }
        }

    private:
        static constexpr const char TAG[] = "ChannelManager";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_MEASUREMENT, TAG >;

        DataSource::Manager & _dataSourceManager;

        Array< Channel, DataSource::Manager::NUM_CHANNELS > _channels;

        void _checkBinding( Channel & channel, uint8_t index )
        {
            channel.bindFlowSensorDataSource( index );
            channel.bindTemperatureSensorDataSource( index );
            channel.bindPressureSensorDataSource( index );
        }
    };
}    // namespace AsnPlus

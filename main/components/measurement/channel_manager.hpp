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
            _database( database ),
            _channels {
                Channel {
                         0, database.channelConfigs[ 0 ],
                         database.channelRuntimes[ 0 ],
                         dataSourceManager, database.eventHistory0,
                         database.channelHistorySeqNums[ 0 ]
                },
                Channel {
                         1, database.channelConfigs[ 1 ],
                         database.channelRuntimes[ 1 ],
                         dataSourceManager, database.eventHistory1,
                         database.channelHistorySeqNums[ 1 ]
                },
                Channel {
                         2, database.channelConfigs[ 2 ],
                         database.channelRuntimes[ 2 ],
                         dataSourceManager, database.eventHistory2,
                         database.channelHistorySeqNums[ 2 ]
                },
                Channel {
                         3, database.channelConfigs[ 3 ],
                         database.channelRuntimes[ 3 ],
                         dataSourceManager, database.eventHistory3,
                         database.channelHistorySeqNums[ 3 ]
                }
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
                if ( channel.getConfig().timestamp != _lastConfigTimestamps[ index ] )
                {
                    Log::info( "Channel %u config changed, reconfiguring", index );
                    _lastConfigTimestamps[ index ] = channel.getConfig().timestamp;
                    _enableFlow( channel, index );
                    _enableTemperature( channel, index );
                    _enablePressure( channel, index );
                    _checkBinding( channel, index );
                }
                channel.poll();

                if ( _database.channelHistorySeqNums[ index ] != _lastSavedSeqNums[ index ] )
                {
                    _lastSavedSeqNums[ index ] = _database.channelHistorySeqNums[ index ];
                    _database.saveChannelHistorySeqNum( index );
                }

                ++index;
            }
        }

    private:
        static constexpr const char TAG[] = "ChannelManager";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_MEASUREMENT, TAG >;

        DataSource::Manager & _dataSourceManager;
        Database &            _database;

        Array< Channel, DataSource::Manager::NUM_CHANNELS > _channels;
        uint64_t _lastConfigTimestamps[ DataSource::Manager::NUM_CHANNELS ] {};
        uint32_t _lastSavedSeqNums[ DataSource::Manager::NUM_CHANNELS ] {};

        void _checkBinding( Channel & channel, uint8_t index )
        {
            channel.bindFlowSensorDataSource( index );
            channel.bindTemperatureSensorDataSource( index );
            channel.bindPressureSensorDataSource( index );
        }

        void _enableFlow( Channel & channel, uint8_t index )
        {
            switch ( channel.getConfig().flowType )
            {
                case Channel::Config::FlowType::MODBUS:
                    {
                        _dataSourceManager.unbindFlowPulseTimerChannel( index );

                        _dataSourceManager.getFlowModbusDataSource( index )->enable();
                        break;
                    }
                case Channel::Config::FlowType::PULSE:
                    {
                        _dataSourceManager.getFlowModbusDataSource( index )->disable();

                        auto port = static_cast< uint8_t >( channel.getConfig().flowConfig.id - 1 );
                        _dataSourceManager.bindFlowPulseToTimerChannel( index, port );
                        break;
                    }
                case Channel::Config::FlowType::UNKNOWN:
                    {
                        _dataSourceManager.getFlowModbusDataSource( index )->disable();

                        _dataSourceManager.unbindFlowPulseTimerChannel( index );
                        break;
                    }
                default:
                    {
                        Log::error(
                            "Unsupported flow type for enabling: %d",
                            static_cast< uint8_t >( channel.getConfig().flowType )
                        );
                        _dataSourceManager.getFlowModbusDataSource( index )->disable();
                        _dataSourceManager.unbindFlowPulseTimerChannel( index );
                        break;
                    }
            }
        }

        void _enableTemperature( Channel & channel, uint8_t index )
        {
            switch ( channel.getConfig().temperatureType )
            {
                case Channel::Config::TemperatureType::MODBUS:
                    {
                        _dataSourceManager.getTemperatureModbusDataSource( index )->enable();
                        break;
                    }
                case Channel::Config::TemperatureType::ANALOG:

                case Channel::Config::TemperatureType::ONE_WIRE:

                case Channel::Config::TemperatureType::UNKNOWN:
                    {
                        _dataSourceManager.getTemperatureModbusDataSource( index )->disable();
                        break;
                    }

                default:
                    {
                        Log::error(
                            "Unsupported temperature type for enabling: %d",
                            static_cast< uint8_t >( channel.getConfig().temperatureType )
                        );
                        _dataSourceManager.getTemperatureModbusDataSource( index )->disable();
                        break;
                    }
            }
        }

        void _enablePressure( Channel & channel, uint8_t index )
        {
            switch ( channel.getConfig().pressureType )
            {
                case Channel::Config::PressureType::ANALOG:
                    {
                        const auto port = static_cast< uint8_t >( channel.getConfig().pressureConfig.id - 1 );
                        _dataSourceManager.bindPressureAnalogToAdcChannel( index, port );
                        break;
                    }
                case Channel::Config::PressureType::UNKNOWN:
                    {
                        _dataSourceManager.unbindPressureAnalogAdcChannel( index );
                        break;
                    }
                default:
                    {
                        Log::error(
                            "Unsupported pressure type for enabling: %d",
                            static_cast< uint8_t >( channel.getConfig().pressureType )
                        );
                        _dataSourceManager.unbindPressureAnalogAdcChannel( index );
                        break;
                    }
            }
        }

        void _enableDataSources()
        {
            uint8_t index = 0;
            for ( auto & channel : _channels )
            {
                _enableFlow( channel, index );
                _enableTemperature( channel, index );
                _enablePressure( channel, index );
                ++index;
            }
        }
    };
}    // namespace AsnPlus

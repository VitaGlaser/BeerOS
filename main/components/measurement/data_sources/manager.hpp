#pragma once

#include "program/components.hpp"

#include "asn/asn-core/array.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "asn/asn-esp32-modbus/master.hpp"
#include "asn/asn-expander-lib/timer/timer.hpp"

#include "flow_modbus.hpp"
#include "flow_pulse.hpp"

#include "temperature_modbus.hpp"

namespace AsnPlus::DataSource
{
    class Manager
    {
    public:
        static constexpr uint8_t  NUM_CHANNELS             = 4;
        static constexpr uint16_t DEFAULT_PULSES_PER_LITRE = 236;

        using PulseChannels                                = Array< Expander::Timers::Timer::Channel, NUM_CHANNELS >;

        Manager( Modbus::RtuMaster & modbusMaster, PulseChannels & channels ) :
            _modbusMaster( modbusMaster ),
            _timerChannels( channels ),
            _flowModbusArray {
                { { modbusMaster }, { modbusMaster }, { modbusMaster }, { modbusMaster } }
        },
            _flowPulseArray {
                { { DEFAULT_PULSES_PER_LITRE },
                  { DEFAULT_PULSES_PER_LITRE },
                  { DEFAULT_PULSES_PER_LITRE },
                  { DEFAULT_PULSES_PER_LITRE } }
            },
            _temperatureModbusArray { { { modbusMaster }, { modbusMaster }, { modbusMaster }, { modbusMaster } } }
        {
        }

        bool initialize()
        {
            Log::debug( "Initializing" );

            _initializeModbusFlows();
            _initializePulseFlows();
            _initializeModbusTemperatures();

            Log::info( "Initialized" );
            return true;
        }

        void poll()
        {
            for ( uint8_t i = 0; i < NUM_CHANNELS; ++i )
            {
                if ( ! _flowModbusArray[ i ].isEnabled() ) continue;

                bool already_polled = false;
                for ( uint8_t j = 0; j < i; ++j )
                {
                    if ( _flowModbusArray[ j ].isEnabled() &&
                         _flowModbusArray[ j ].getId() == _flowModbusArray[ i ].getId() )
                    {
                        already_polled = true;
                        break;
                    }
                }

                if ( ! already_polled ) _flowModbusArray[ i ].poll();
            }

            for ( uint8_t i = 0; i < NUM_CHANNELS; ++i )
            {
                if ( _flowPulseArray[ i ].isEnabled() ) _flowPulseArray[ i ].poll();
            }

            for ( uint8_t i = 0; i < NUM_CHANNELS; ++i )
            {
                if ( ! _temperatureModbusArray[ i ].isEnabled() ) continue;

                bool already_polled = false;
                for ( uint8_t j = 0; j < i; ++j )
                {
                    if ( _temperatureModbusArray[ j ].isEnabled() &&
                         _temperatureModbusArray[ j ].getId() == _temperatureModbusArray[ i ].getId() )
                    {
                        already_polled = true;
                        break;
                    }
                }

                if ( ! already_polled ) _temperatureModbusArray[ i ].poll();
            }
        }

        FlowModbus * getFlowModbusDataSource( uint8_t index )
        {
            if ( index >= NUM_CHANNELS )
            {
                Log::error( "Invalid index for FlowModbus data source: %d", index );
                return nullptr;
            }

            for ( uint8_t j = 0; j < index; ++j )
            {
                if ( _flowModbusArray[ j ].isEnabled() &&
                     _flowModbusArray[ j ].getId() == _flowModbusArray[ index ].getId() )
                    return &_flowModbusArray[ j ];
            }

            return &_flowModbusArray[ index ];
        }

        FlowModbus * getFlowModbusDataSourceById( uint64_t id )
        {
            for ( auto & source : _flowModbusArray )
            {
                if ( source.getId() == id ) return &source;
            }

            return nullptr;
        }

        FlowPulse * getFlowPulseDataSource( uint8_t index )
        {
            if ( index >= NUM_CHANNELS )
            {
                Log::error( "Invalid index for FlowPulse data source: %d", index );
                return nullptr;
            }
            return &_flowPulseArray[ index ];
        }

        void bindFlowPulseToTimerChannel( uint8_t channel_index, uint8_t port )
        {
            if ( channel_index >= NUM_CHANNELS || port >= NUM_CHANNELS )
            {
                Log::error( "Invalid channel_index %u or port %u for FlowPulse binding", channel_index, port );
                return;
            }
            _flowPulseArray[ channel_index ].bindTimerChannel( _timerChannels[ port ] );
            _flowPulseArray[ channel_index ].enable();
        }

        void unbindFlowPulseTimerChannel( uint8_t channel_index )
        {
            if ( channel_index >= NUM_CHANNELS ) return;
            _flowPulseArray[ channel_index ].disable();
            _flowPulseArray[ channel_index ].unbindTimerChannel();
        }

        TemperatureModbus * getTemperatureModbusDataSource( uint8_t index )
        {
            if ( index >= NUM_CHANNELS )
            {
                Log::error( "Invalid index for TemperatureModbus data source: %d", index );
                return nullptr;
            }

            for ( uint8_t j = 0; j < index; ++j )
            {
                if ( _temperatureModbusArray[ j ].isEnabled() &&
                     _temperatureModbusArray[ j ].getId() == _temperatureModbusArray[ index ].getId() )
                    return &_temperatureModbusArray[ j ];
            }

            return &_temperatureModbusArray[ index ];
        }

        TemperatureModbus * getTemperatureModbusDataSourceById( uint64_t id )
        {
            for ( auto & source : _temperatureModbusArray )
            {
                if ( source.getId() == id ) return &source;
            }

            return nullptr;
        }

    private:
        static constexpr const char TAG[] = "DataSourcesManager";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_DATA_SOURCES, TAG >;

        Modbus::RtuMaster & _modbusMaster;

        PulseChannels _timerChannels;

        Array< FlowModbus, NUM_CHANNELS >        _flowModbusArray;
        Array< FlowPulse, NUM_CHANNELS >         _flowPulseArray;
        Array< TemperatureModbus, NUM_CHANNELS > _temperatureModbusArray;

        void _initializeModbusFlows()
        {
            for ( auto & source : _flowModbusArray ) source.initialize();
        }

        void _initializePulseFlows()
        {
            for ( auto & source : _flowPulseArray ) source.initialize();
        }

        void _initializeModbusTemperatures()
        {
            for ( auto & source : _temperatureModbusArray ) source.initialize();
        }
    };
}    // namespace AsnPlus::DataSource

#pragma once

#include "program/components.hpp"

#include "asn/asn-core/array.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "asn/asn-esp32-modbus/include/master.hpp"
#include "asn/asn-expander-lib/include/adc/adc.hpp"
#include "asn/asn-expander-lib/include/timer/timer.hpp"

#include "flow_modbus.hpp"
#include "flow_pulse.hpp"
#include "temperature_modbus.hpp"
#include "temperature_analog.hpp"
#include "temperature_onewire.hpp"
#include "pressure_analog.hpp"
#include "conductivity_modbus.hpp"

namespace AsnPlus::DataSource
{
    class Manager
    {
    public:
        static constexpr uint8_t  NUM_CHANNELS             = 4;
        static constexpr uint16_t DEFAULT_PULSES_PER_LITRE = 236;

        using PulseChannels = Array< Expander::Timers::Timer::Channel, NUM_CHANNELS >;
        using AdcChannels   = Array< Expander::Adc::Adc::Channel, NUM_CHANNELS >;

        Manager( Modbus::RtuMaster & modbusMaster, PulseChannels & channels, AdcChannels & adcChannels ) :
            _modbusMaster( modbusMaster ),
            _timerChannels( channels ),
            _adcChannels( adcChannels ),
            _flowModbusArray { { { modbusMaster }, { modbusMaster }, { modbusMaster }, { modbusMaster } } },
            _flowPulseArray {
                { { DEFAULT_PULSES_PER_LITRE },
                  { DEFAULT_PULSES_PER_LITRE },
                  { DEFAULT_PULSES_PER_LITRE },
                  { DEFAULT_PULSES_PER_LITRE } }
            },
            _temperatureModbusArray { { { modbusMaster }, { modbusMaster }, { modbusMaster }, { modbusMaster } } },
            _conductivityModbusArray { { { modbusMaster }, { modbusMaster }, { modbusMaster }, { modbusMaster } } }
        {
        }

        bool initialize()
        {
            Log::debug( "Initializing" );

            _initializeModbusFlows();
            _initializePulseFlows();
            _initializeModbusTemperatures();
            _initializeAnalogTemperatures();
            _initializeOneWireTemperatures();
            _initializeAnalogPressures();
            _initializeModbusConductivities();

            Log::info( "Initialized" );
            return true;
        }

        void pollModbus()
        {
            _pollModbusArray( _flowModbusArray );
            _pollModbusArray( _temperatureModbusArray );
            _pollModbusArray( _conductivityModbusArray );
        }

        void pollPulse()
        {
            for ( uint8_t i = 0; i < NUM_CHANNELS; ++i )
            {
                if ( _flowPulseArray[ i ].isEnabled() ) _flowPulseArray[ i ].poll();
            }
        }

        void pollAnalog()
        {
            for ( uint8_t i = 0; i < NUM_CHANNELS; ++i )
            {
                if ( _temperatureAnalogArray[ i ].isEnabled() ) _temperatureAnalogArray[ i ].poll();
            }

            for ( uint8_t i = 0; i < NUM_CHANNELS; ++i )
            {
                if ( _pressureAnalogArray[ i ].isEnabled() ) _pressureAnalogArray[ i ].poll();
            }
        }

        void pollOnewire()
        {
            if ( _temperatureOneWire.isEnabled() ) _temperatureOneWire.poll();
        }

        void poll()
        {
            pollModbus();
            pollPulse();
            pollAnalog();
            pollOnewire();
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

        void bindPressureAnalogToAdcChannel( uint8_t channel_index, uint8_t port )
        {
            if ( channel_index >= NUM_CHANNELS || port >= NUM_CHANNELS )
            {
                Log::error( "Invalid channel_index %u or port %u for PressureAnalog binding", channel_index, port );
                return;
            }
            _pressureAnalogArray[ channel_index ].bindAdcChannel( _adcChannels[ port ] );
            _pressureAnalogArray[ channel_index ].enable();
        }

        void unbindPressureAnalogAdcChannel( uint8_t channel_index )
        {
            if ( channel_index >= NUM_CHANNELS ) return;
            _pressureAnalogArray[ channel_index ].disable();
            _pressureAnalogArray[ channel_index ].unbindAdcChannel();
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

        TemperatureAnalog * getTemperatureAnalogDataSource( uint8_t index )
        {
            if ( index >= NUM_CHANNELS )
            {
                Log::error( "Invalid index for TemperatureAnalog data source: %d", index );
                return nullptr;
            }
            return &_temperatureAnalogArray[ index ];
        }

        TemperatureOneWire * getTemperatureOneWireDataSource() { return &_temperatureOneWire; }

        PressureAnalog * getPressureAnalogDataSource( uint8_t index )
        {
            if ( index >= NUM_CHANNELS )
            {
                Log::error( "Invalid index for PressureAnalog data source: %d", index );
                return nullptr;
            }
            return &_pressureAnalogArray[ index ];
        }

        ConductivityModbus * getConductivityModbusDataSource( uint8_t index )
        {
            if ( index >= NUM_CHANNELS )
            {
                Log::error( "Invalid index for ConductivityModbus data source: %d", index );
                return nullptr;
            }

            for ( uint8_t j = 0; j < index; ++j )
            {
                if ( _conductivityModbusArray[ j ].isEnabled() &&
                     _conductivityModbusArray[ j ].getId() == _conductivityModbusArray[ index ].getId() )
                    return &_conductivityModbusArray[ j ];
            }

            return &_conductivityModbusArray[ index ];
        }

        ConductivityModbus * getConductivityModbusDataSourceById( uint64_t id )
        {
            for ( auto & source : _conductivityModbusArray )
            {
                if ( source.getId() == id ) return &source;
            }

            return nullptr;
        }

    private:
        static constexpr const char TAG[] = "DataSourcesManager";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_DATA_SOURCES, TAG >;

        Modbus::RtuMaster & _modbusMaster;
        PulseChannels       _timerChannels;
        AdcChannels         _adcChannels;

        Array< FlowModbus, NUM_CHANNELS >          _flowModbusArray;
        Array< FlowPulse, NUM_CHANNELS >           _flowPulseArray;
        Array< TemperatureModbus, NUM_CHANNELS >   _temperatureModbusArray;
        Array< TemperatureAnalog, NUM_CHANNELS >   _temperatureAnalogArray;
        TemperatureOneWire                         _temperatureOneWire;
        Array< PressureAnalog, NUM_CHANNELS >      _pressureAnalogArray;
        Array< ConductivityModbus, NUM_CHANNELS >  _conductivityModbusArray;

        template< typename ARRAY >
        void _pollModbusArray( ARRAY & array )
        {
            for ( uint8_t i = 0; i < NUM_CHANNELS; ++i )
            {
                if ( ! array[ i ].isEnabled() ) continue;

                bool already_polled = false;
                for ( uint8_t j = 0; j < i; ++j )
                {
                    if ( array[ j ].isEnabled() && array[ j ].getId() == array[ i ].getId() )
                    {
                        already_polled = true;
                        break;
                    }
                }

                if ( ! already_polled ) array[ i ].poll();
            }
        }

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

        void _initializeAnalogTemperatures()
        {
            for ( auto & source : _temperatureAnalogArray ) source.initialize();
        }

        void _initializeOneWireTemperatures() { _temperatureOneWire.initialize(); }

        void _initializeAnalogPressures()
        {
            for ( auto & source : _pressureAnalogArray ) source.initialize();
        }

        void _initializeModbusConductivities()
        {
            for ( auto & source : _conductivityModbusArray ) source.initialize();
        }
    };
}    // namespace AsnPlus::DataSource

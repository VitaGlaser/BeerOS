#pragma once

#include "program/components.hpp"

#include "asn/asn-core/array.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "asn/asn-esp32-modbus/master.hpp"

#include "flow_modbus.hpp"

#include "temperature_modbus.hpp"

namespace AsnPlus::DataSource
{
    class Manager
    {
    public:
        static constexpr uint8_t NUM_CHANNELS = 4;

        Manager( Modbus::RtuMaster & modbusMaster ) :
            _modbusMaster( modbusMaster ),
            _flowModbusArray { { { modbusMaster }, { modbusMaster }, { modbusMaster }, { modbusMaster } } },
            _temperatureModbusArray { { { modbusMaster }, { modbusMaster }, { modbusMaster }, { modbusMaster } } }
        {
        }

        bool initialize()
        {
            Log::debug( "Initializing" );

            _initializeModbusFlows();

            _initializeModbusTemperatures();

            Log::info( "Initialized" );
            return true;
        }

        FlowModbus * getFlowModbusDataSource( uint8_t index )
        {
            if ( index >= NUM_CHANNELS )
            {
                Log::error( "Invalid index for FlowModbus data source: %d", index );
                return nullptr;
            }

            return &_flowModbusArray[ index ];
        }

        TemperatureModbus * getTemperatureModbusDataSource( uint8_t index )
        {
            if ( index >= NUM_CHANNELS )
            {
                Log::error( "Invalid index for TemperatureModbus data source: %d", index );
                return nullptr;
            }

            return &_temperatureModbusArray[ index ];
        }

    private:
        static constexpr const char TAG[] = "DataSourcesManager";
        using Log                         = Logger< ProgramConfig::DEBUG_ENABLED, TAG >;

        Modbus::RtuMaster & _modbusMaster;

        Array< FlowModbus, NUM_CHANNELS > _flowModbusArray;

        Array< TemperatureModbus, NUM_CHANNELS > _temperatureModbusArray;

        void _initializeModbusFlows()
        {
            for ( auto & channel : _flowModbusArray )
            {
                channel.initialize();
            }
        }

        void _initializeModbusTemperatures()
        {
            for ( auto & channel : _temperatureModbusArray )
            {
                channel.initialize();
            }
        }
    };
}    // namespace AsnPlus::DataSource

#pragma once

#include "program/config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "event_monitor.hpp"
#include "sensor.hpp"

#include "data_sources/manager.hpp"

namespace AsnPlus
{
    class Channel
    {
    public:
        using Runtime = EventMonitor::Runtime;

        struct Config : AsnPlus::Config
        {
            enum class FlowType : uint8_t
            {
                UNKNOWN = 0,
                MODBUS,
                PULSE
            };

            enum class TemperatureType : uint8_t
            {
                UNKNOWN = 0,
                MODBUS,
                ANALOG,
                ONE_WIRE
            };

            enum class PressureType : uint8_t
            {
                UNKNOWN = 0,
                ANALOG
            };

            struct BeerSizeConfig
            {
                static constexpr size_t NAME_LENGTH = 32;

                char     name[ NAME_LENGTH ] {};
                uint32_t minVolume = 0;
                uint32_t typVolume = 0;
                uint32_t maxVolume = 0;
            };

            static constexpr size_t   PORT_CONFIG_ID_LENGTH = 32;
            static constexpr uint8_t  BEER_SIZE_COUNT       = 8;

            char            portConfigId[ PORT_CONFIG_ID_LENGTH ] {};
            bool            enabled  = false;
            FlowType        flowType = FlowType::UNKNOWN;
            Sensor::Config  flowConfig {};
            TemperatureType temperatureType = TemperatureType::UNKNOWN;
            Sensor::Config  temperatureConfig {};
            PressureType    pressureType = PressureType::UNKNOWN;
            Sensor::Config  pressureConfig {};
            uint16_t        beerTypeId = 0;
            BeerSizeConfig  beerSizes[ BEER_SIZE_COUNT ] {};
            uint32_t        tapTimeoutMs = 0;
            uint32_t        tankCapacity = 0;
            uint32_t        cleaningVolumeThr = 0;
        };

        Channel(
            Config &                        config,
            EventMonitor::Runtime &         runtime,
            DataSource::Manager &           dataSourceManager,
            IRingBuffer< EventMonitor::Event > & event_history
        ) :
            _config( config ),
            _runtime( runtime ),
            _dataSourceManager( dataSourceManager ),
            _flow( config.flowConfig ),
            _temperature( config.temperatureConfig ),
            _pressure( config.pressureConfig ),
            _eventMonitor( _config.tapTimeoutMs, _runtime, _flow, _temperature, _pressure, event_history )
        {
        }

        bool initialize()
        {
            Log::debug( "Initializing" );

            _flow.initialize();
            _temperature.initialize();
            _pressure.initialize();

            Log::info( "Initialized" );
            return true;
        }

        void poll()
        {
            if ( _config.flowType != Config::FlowType::UNKNOWN )
            {
                _flow.poll();
            }

            if ( _config.temperatureType != Config::TemperatureType::UNKNOWN )
            {
                _temperature.poll();
            }

            if ( _config.pressureType != Config::PressureType::UNKNOWN )
            {
                _pressure.poll();
            }
        }

        Config & getConfig() { return _config; }

        Runtime & getRuntime() { return _runtime; }

        void bindFlowSensorDataSource( uint8_t index )
        {
            switch ( _config.flowType )
            {
                case Config::FlowType::MODBUS:
                    if ( auto * dataSource = _dataSourceManager.getFlowModbusDataSource( index ) )
                    {
                        dataSource->setId( _config.flowConfig.id );
                        _flow.bindDataSource( *dataSource );
                    }
                    break;
                case Config::FlowType::PULSE:
                    // if ( auto dataSource = _dataSourceManager.getPulseDataSource( _config.flowConfig.id ) )
                    // {
                    //     _flow.bindDataSource( *dataSource );
                    // }
                    // break;
                case Config::FlowType::UNKNOWN:
                    _flow.unbindDataSource();
                    break;

                default:
                    Log::error( "Unsupported flow type for binding: %d", static_cast< uint8_t >( _config.flowType ) );
            }
        }

        void bindTemperatureSensorDataSource( uint8_t index )
        {
            switch ( _config.temperatureType )
            {
                case Config::TemperatureType::MODBUS:
                    if ( auto * dataSource = _dataSourceManager.getTemperatureModbusDataSource( index ) )
                    {
                        dataSource->setId( _config.temperatureConfig.id );
                        _temperature.bindDataSource( *dataSource );
                    }
                    break;
                case Config::TemperatureType::ANALOG:
                    // if ( auto dataSource =
                    //          _dataSourceManager.getAnalogTemperatureDataSource( _config.temperatureConfig.id ) )
                    // {
                    //     _temperature.bindDataSource( *dataSource );
                    // }
                    // break;
                case Config::TemperatureType::ONE_WIRE:
                    // if ( auto dataSource = _dataSourceManager.getOneWireTemperatureDataSource( index ) )
                    // {
                    //     _temperature.bindDataSource( *dataSource );
                    //     _datasource.setId( _config.temperatureConfig.id );
                    // }
                    // break;
                case Config::TemperatureType::UNKNOWN:
                    _temperature.unbindDataSource();
                    break;

                default:
                    Log::error(
                        "Unsupported temperature type for binding: %d",
                        static_cast< uint8_t >( _config.temperatureType )
                    );
            }
        }

        void bindPressureSensorDataSource( uint8_t index )
        {
            switch ( _config.pressureType )
            {
                case Config::PressureType::ANALOG:
                    // if ( auto dataSource = _dataSourceManager.getAnalogPressureDataSource( _config.pressureConfig.id
                    // ) )
                    // {
                    //     _pressure.bindDataSource( *dataSource );
                    // }
                    // break;
                case Config::PressureType::UNKNOWN:
                    _pressure.unbindDataSource();
                    break;

                default:
                    Log::error(
                        "Unsupported pressure type for binding: %d", static_cast< uint8_t >( _config.pressureType )
                    );
            }
        }

    private:
        static constexpr const char TAG[] = "Channel";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_MEASUREMENT, TAG >;

        Config &  _config;
        Runtime & _runtime;

        DataSource::Manager & _dataSourceManager;

        Sensor _flow;
        Sensor _temperature;
        Sensor _pressure;

        EventMonitor _eventMonitor;
    };
}    // namespace AsnPlus

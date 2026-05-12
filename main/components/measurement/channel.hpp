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

            enum class ConductivityType : uint8_t
            {
                UNKNOWN = 0,
                MODBUS
            };

            struct ClassificationConfig
            {
                static constexpr size_t NAME_LENGTH = 32;

                char     name[ NAME_LENGTH ] {};
                uint32_t minVolume = 0;
                uint32_t typVolume = 0;
                uint32_t maxVolume = 0;
            };

            static constexpr uint8_t CLASSIFICATION_CONFIG_COUNT                      = 8;

            bool                 enabled                                              = false;
            FlowType             flowType                                             = FlowType::UNKNOWN;
            Sensor::Config       flowConfig                                           = {};
            TemperatureType      temperatureType                                      = TemperatureType::UNKNOWN;
            Sensor::Config       temperatureConfig                                    = {};
            PressureType         pressureType                                         = PressureType::UNKNOWN;
            Sensor::Config       pressureConfig                                       = {};
            ConductivityType     conductivityType                                     = ConductivityType::UNKNOWN;
            Sensor::Config       conductivityConfig                                   = {};
            uint16_t             beverageId                                           = 0;
            ClassificationConfig classificationConfigs[ CLASSIFICATION_CONFIG_COUNT ] = {};
            uint32_t             tapTimeoutMs                                         = 0;
            uint32_t             tankCapacity                                         = 0;
            uint32_t             cleaningVolumeThr                                    = 0;
        };

        struct UnrecognizedEvents
        {
            uint32_t count  = 0;
            uint32_t volume = 0;
        };

        struct ClassificationState
        {
            uint32_t count           = 0;
            uint32_t volume          = 0;
            uint32_t averageQuality  = 0;    // TBD
            uint32_t underLimitCount = 0;
            uint32_t overLimitCount  = 0;
        };

        struct Runtime : AsnPlus::Runtime
        {
            uint64_t tankLevel                                                             = 0;    // ml
            uint16_t flow                                                                  = 0;    // ml/min
            uint16_t temperature                                                           = 0;    // celsius * 10
            uint16_t pressure                                                              = 0;
            uint16_t conductivity                                                          = 0;    // TBD

            UnrecognizedEvents unrecognizedEvents                                          = {};

            ClassificationState classificationState[ Config::CLASSIFICATION_CONFIG_COUNT ] = {};
        };

        Channel(
            uint8_t                              index,
            Config &                             config,
            Runtime &                            runtime,
            DataSource::Manager &                dataSourceManager,
            IRingBuffer< EventMonitor::Event > & eventHistory,
            uint32_t &                           lastSeqNum
        ) :
        _index( index ),
            _config( config ),
            _runtime( runtime ),
            _dataSourceManager( dataSourceManager ),
            _lastSeqNum( lastSeqNum ),
            _flow( config.flowConfig ),
            _temperature( config.temperatureConfig ),
            _pressure( config.pressureConfig ),
            _eventMonitor(
                _config.tapTimeoutMs,
                _eventMonitorRuntime,
                _flow,
                _temperature,
                _pressure,
                eventHistory,
                _lastSeqNum,
                Delegate< void( EventMonitor::Event & ) >::create< Channel, &Channel::_onEventEnd >( *this )
            )
        {
        }

        bool initialize()
        {
            Log::debug( "Initializing (%u)", _index );

            _flow.initialize();
            _temperature.initialize();
            _pressure.initialize();

            Log::info( "Initialized (%u)", _index );
            return true;
        }

        void poll()
        {
            if ( ! _config.enabled ) return;

            _eventMonitor.poll();

            _runtime.flow        = _eventMonitorRuntime.flow.value;
            _runtime.temperature = _eventMonitorRuntime.temperature.value;
            _runtime.pressure    = _eventMonitorRuntime.pressure.value;
        }

        Config & getConfig() { return _config; }

        Runtime & getRuntime() { return _runtime; }

        void resetRuntime() { memset( &_runtime, 0, sizeof( Runtime ) ); }

        void bindFlowSensorDataSource( uint8_t index )
        {
            switch ( _config.flowType )
            {
                case Config::FlowType::MODBUS:
                    {
                        auto * dataSource = _dataSourceManager.getFlowModbusDataSourceById( _config.flowConfig.id );
                        if ( dataSource )
                        {
                            Log::debug( " (%u) Binding Modbus flow sensor data source by ID '%llu'", _index, _config.flowConfig.id );
                        }
                        else
                        {
                            Log::debug(
                                " (%u) No Modbus flow data source with ID '%llu', falling back to index %u",
                                _index,
                                _config.flowConfig.id,
                                index
                            );
                            dataSource = _dataSourceManager.getFlowModbusDataSource( index );
                            if ( dataSource ) dataSource->setId( _config.flowConfig.id );
                        }
                        if ( dataSource ) _flow.bindDataSource( *dataSource );
                        break;
                    }
                case Config::FlowType::PULSE:
                    {
                        auto * dataSource = _dataSourceManager.getFlowPulseDataSource( index );
                        if ( dataSource )
                        {
                            Log::debug(
                                " (%u) Binding pulse flow sensor (channel %u -> port %llu)", _index, index, _config.flowConfig.id
                            );
                            _flow.bindDataSource( *dataSource );
                        }
                        break;
                    }
                case Config::FlowType::UNKNOWN:
                    _flow.unbindDataSource();
                    break;

                default:
                    Log::error( " (%u) Unsupported flow type for binding: %d", _index, static_cast< uint8_t >( _config.flowType ) );
            }
        }

        void bindTemperatureSensorDataSource( uint8_t index )
        {
            switch ( _config.temperatureType )
            {
                case Config::TemperatureType::MODBUS:
                    {
                        auto * dataSource =
                            _dataSourceManager.getTemperatureModbusDataSourceById( _config.temperatureConfig.id );
                        if ( dataSource )
                        {
                            Log::debug(
                                " (%u) Binding Modbus temperature sensor data source by ID '%llu'", _index, _config.temperatureConfig.id
                            );
                        }
                        else
                        {
                            Log::debug(
                                " (%u) No Modbus temperature data source with ID '%llu', falling back to index %u", _index, _config.temperatureConfig.id, index
                            );
                            dataSource = _dataSourceManager.getTemperatureModbusDataSource( index );
                            if ( dataSource ) dataSource->setId( _config.temperatureConfig.id );
                        }
                        if ( dataSource ) _temperature.bindDataSource( *dataSource );
                        break;
                    }
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
                        " (%u) Unsupported temperature type for binding: %d", _index, static_cast< uint8_t >( _config.temperatureType )
                    );
            }
        }

        void bindPressureSensorDataSource( uint8_t index )
        {
            switch ( _config.pressureType )
            {
                case Config::PressureType::ANALOG:
                    {
                        auto * dataSource = _dataSourceManager.getPressureAnalogDataSource( index );
                        if ( dataSource )
                        {
                            Log::debug( " (%u) Binding analog pressure sensor data source (index %u)", _index, index );
                            _pressure.bindDataSource( *dataSource );
                        }
                        break;
                    }
                case Config::PressureType::UNKNOWN:
                    _pressure.unbindDataSource();
                    break;

                default:
                    Log::error(
                        " (%u) Unsupported pressure type for binding: %d", _index, static_cast< uint8_t >( _config.pressureType )
                    );
            }
        }

    private:
        static constexpr const char TAG[] = "Channel";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_CHANNEL, TAG >;

        uint8_t _index;

        Config &  _config;
        Runtime & _runtime;

        DataSource::Manager & _dataSourceManager;

        uint32_t & _lastSeqNum;

        Sensor _flow;
        Sensor _temperature;
        Sensor _pressure;

        EventMonitor::Runtime _eventMonitorRuntime;
        EventMonitor          _eventMonitor;

        int8_t _determineBeerSizeIndex( uint32_t volume ) const
        {
            for ( uint8_t i = 0; i < Config::CLASSIFICATION_CONFIG_COUNT; ++i )
            {
                const auto & size = _config.classificationConfigs[ i ];
                if ( size.minVolume == 0 && size.maxVolume == 0 ) continue;
                if ( volume >= size.minVolume && volume <= size.maxVolume )
                {
                    return static_cast< int8_t >( i );
                }
            }
            return -1;
        }

        void _onEventEnd( EventMonitor::Event & event )
        {
            // TODO(DK): This should categorize beverage in the event and add the information the eventMonitor doesnt
            // know about - like beverage type and size
            if ( _config.cleaningVolumeThr > 0 && event.volume >= _config.cleaningVolumeThr )
            {
                event.type = EventMonitor::Event::EventType::CLEANING;
                Log::info(
                    " (%u) Event classified as cleaning (volume=%u, threshold=%u)", _index, event.volume, _config.cleaningVolumeThr
                );
                return;
            }

            const int8_t index = _determineBeerSizeIndex( event.volume );
            if ( index < 0 )
            {
                event.type = EventMonitor::Event::EventType::UNKNOWN;
                Log::warn(
                    " (%u) Event volume %u does not match any classification, counting as unrecognized", _index, event.volume
                );
                ++_runtime.unrecognizedEvents.count;
                _runtime.unrecognizedEvents.volume += event.volume;
                return;
            }

            event.type = EventMonitor::Event::EventType::BEVERAGE;
            Log::info(
                " (%u) Event assigned to classification index %d (volume=%u)", _index, static_cast< int >( index ), event.volume
            );
            auto & state = _runtime.classificationState[ static_cast< uint8_t >( index ) ];
            ++state.count;
            state.volume += event.volume;
        }
    };
}    // namespace AsnPlus

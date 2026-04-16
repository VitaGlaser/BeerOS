#pragma once

#include "program/config.hpp"

#include <cstring>

#include "esp_heap_caps.h"

#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/ring_buffer.hpp"
#include "asn/asn-core/types.hpp"

#include "sensor.hpp"

namespace AsnPlus
{
    class EventMonitor
    {
    public:
        static constexpr uint8_t MAX_EVENT_HISTORY = 100;

        struct Runtime : AsnPlus::Runtime
        {
            DataSource::Base::Sample flow {};
            DataSource::Base::Sample temperature {};
            DataSource::Base::Sample pressure {};
        };

        struct Event
        {
            static constexpr uint16_t HISTORY_SIZE = 100;

            enum class EventType : uint8_t
            {
                UNKNOWN,
                BEVERAGE,
                CLEANING
            };

            uint64_t configTimestamp = 0;

            uint64_t  sequenceNumber = 0;
            uint64_t  startTimestamp = 0;
            uint64_t  endTimestamp   = 0;
            bool      synced         = false;
            EventType type           = EventType::UNKNOWN;
            uint32_t  volume         = 0;

            uint8_t classification   = 0;

            uint16_t avgTemperature  = 0;
            uint16_t avgConductivity = 0;

            uint32_t flowProfile[ HISTORY_SIZE ] {};
            uint32_t pressureProfile[ HISTORY_SIZE ] {};
        };

        EventMonitor(
            uint32_t &                  tapTimeoutMs,
            Runtime &                   runtime,
            Sensor &                    flow,
            Sensor &                    temperature,
            Sensor &                    pressure,
            IRingBuffer< Event > &      eventHistory,
            Delegate< void( Event & ) > onEventEnd = {}
        ) :
            _tapTimeoutMs( tapTimeoutMs ),
            _runtime( runtime ),
            _flow( flow ),
            _temperature( temperature ),
            _pressure( pressure ),
            _eventHistory( eventHistory ),
            _onEventEnd( onEventEnd )
        {
        }

        bool initialize()
        {
            Log::debug( "Initializing" );

            if ( _currentEvent == nullptr )
            {
                Log::error( "Failed to allocate event buffer in PSRAM" );
                return false;
            }

            Log::info( "Initialized" );
            return true;
        }

        void poll()
        {
            if ( _tapTimeoutMs == 0 )
            {
                Log::debug( "Tap timeout is 0, skipping event monitoring" );
                return;
            }

            _runtime.flow        = _flow.read();
            _runtime.temperature = _temperature.read();
            _runtime.pressure    = _pressure.read();

            if ( ! _eventActive )
            {
                if ( _runtime.flow.value == 0 )
                {
                    return;
                }

                startEvent( _runtime.flow.timestamp );
            }

            if ( _eventActive )
            {
                _addSample( _runtime.flow, _runtime.temperature, _runtime.pressure );

                if ( _runtime.flow.value == 0 )
                {
                    if ( _zeroFlowStartMs == 0 )
                    {
                        _zeroFlowStartMs = _runtime.flow.timestamp;
                    }
                    else if ( _runtime.flow.timestamp - _zeroFlowStartMs >= _tapTimeoutMs )
                    {
                        Log::info( "Tap timeout reached, ending event" );
                        endEvent( _runtime.flow.timestamp );
                    }
                }
                else
                {
                    _zeroFlowStartMs = 0;
                }
            }
        }

        bool startEvent( uint64_t timestamp )
        {
            Log::debug( "Starting event" );
            if ( _eventActive )
            {
                Log::warn( "Event is already active" );
                return false;
            }

            _eventActive = true;

            memset( _currentEvent, 0, sizeof( Event ) );
            _currentEvent->sequenceNumber = _eventHistory.empty() ? 1 : _eventHistory.back().sequenceNumber + 1;
            _currentEvent->startTimestamp = timestamp;

            _volumeAcc                    = 0;
            _prevFlowSample               = {};
            _dsWindowStart                = 0;
            _dsFlowAcc                    = 0;
            _dsTempAcc                    = 0;
            _dsPressAcc                   = 0;
            _dsCount                      = 0;
            _dsOutIdx                     = 0;

            Log::info( "Event started" );
            return true;
        }

        bool endEvent( uint64_t timestamp )
        {
            Log::debug( "Ending event" );

            if ( ! _eventActive )
            {
                Log::warn( "Event is not active" );
                return false;
            }

            _eventActive     = false;
            _zeroFlowStartMs = 0;

            // Finalize volume: add contribution from last sample to endTimestamp
            if ( _prevFlowSample.timestamp != 0 )
            {
                uint64_t dt = timestamp - _prevFlowSample.timestamp;
                _volumeAcc += static_cast< uint64_t >( _prevFlowSample.value ) * dt;
            }

            // Flush remaining downsample window
            _flushDownsampleWindow();

            _currentEvent->endTimestamp = timestamp;
            _currentEvent->volume       = static_cast< uint32_t >( _volumeAcc / MS_PER_MINUTE );
            _eventHistory.push( *_currentEvent );

            Log::info(
                "Event: startTimestamp=%llu, endTimestamp=%llu, volume=%u, historySlots=%u",
                _currentEvent->startTimestamp,
                _currentEvent->endTimestamp,
                _currentEvent->volume,
                static_cast< uint32_t >( _dsOutIdx )
            );

            if ( _onEventEnd.is_valid() )
            {
                _onEventEnd( *_currentEvent );
            }

            memset( _currentEvent, 0, sizeof( Event ) );

            Log::info( "Event ended" );
            return true;
        }

    private:
        static constexpr const char TAG[]              = "EventMonitor";
        using Log                                      = Logger< ProjectConfig::LOG_LEVEL_MEASUREMENT, TAG >;

        static constexpr uint32_t MS_PER_MINUTE        = 60'000;
        static constexpr uint32_t DOWNSAMPLE_WINDOW_MS = 200;

        uint32_t & _tapTimeoutMs;
        Runtime &  _runtime;

        bool     _eventActive     = false;
        uint64_t _zeroFlowStartMs = 0;

        Sensor & _flow;
        Sensor & _temperature;
        Sensor & _pressure;

        Event * _currentEvent =
            static_cast< Event * >( heap_caps_calloc( 1, sizeof( Event ), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT ) );

        IRingBuffer< Event > &      _eventHistory;
        Delegate< void( Event & ) > _onEventEnd;

        // MARK: IncrementalVolume
        uint64_t                 _volumeAcc      = 0;
        DataSource::Base::Sample _prevFlowSample = {};

        // MARK: IncrementalDownsampling
        uint64_t _dsWindowStart                  = 0;
        uint64_t _dsFlowAcc                      = 0;
        uint64_t _dsTempAcc                      = 0;
        uint64_t _dsPressAcc                     = 0;
        uint32_t _dsCount                        = 0;
        uint16_t _dsOutIdx                       = 0;

        void _addSample(
            const DataSource::Base::Sample & flow,
            const DataSource::Base::Sample & temp,
            const DataSource::Base::Sample & press
        )
        {
            // Incremental volume: accumulate (prevValue * dt) for each new sample
            if ( _prevFlowSample.timestamp != 0 )
            {
                uint64_t dt = flow.timestamp - _prevFlowSample.timestamp;
                _volumeAcc += static_cast< uint64_t >( _prevFlowSample.value ) * dt;
            }
            _prevFlowSample = flow;

            // Incremental downsampling: accumulate into current window
            if ( _dsCount == 0 )
            {
                _dsWindowStart = flow.timestamp;
            }

            _dsFlowAcc  += flow.value;
            _dsTempAcc  += temp.value;
            _dsPressAcc += press.value;
            ++_dsCount;

            if ( flow.timestamp - _dsWindowStart >= DOWNSAMPLE_WINDOW_MS )
            {
                _flushDownsampleWindow();
                _dsWindowStart = flow.timestamp;
            }
        }

        void _flushDownsampleWindow()
        {
            if ( _dsCount == 0 || _dsOutIdx >= Event::HISTORY_SIZE ) return;

            // TODO(DK): Add average temperature and conductivity to event

            _currentEvent->flowProfile[ _dsOutIdx ]     = static_cast< uint32_t >( _dsFlowAcc / _dsCount );
            _currentEvent->pressureProfile[ _dsOutIdx ] = static_cast< uint32_t >( _dsPressAcc / _dsCount );
            ++_dsOutIdx;

            _dsFlowAcc  = 0;
            _dsTempAcc  = 0;
            _dsPressAcc = 0;
            _dsCount    = 0;
        }
    };
}    // namespace AsnPlus

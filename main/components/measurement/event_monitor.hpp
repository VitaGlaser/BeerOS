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
            uint64_t  pulseCount     = 0;
            bool      discarded      = false;

            uint8_t classification   = 0;

            uint16_t avgTemperature  = 0;
            uint16_t avgConductivity = 0;

            uint32_t flowProfile[ HISTORY_SIZE ] {};
            uint32_t volumeFlowProfile[ HISTORY_SIZE ] {};
            uint32_t volumePulseProfile[ HISTORY_SIZE ] {};
            uint32_t pulseProfile[ HISTORY_SIZE ] {};
        };

        EventMonitor(
            uint32_t &                  tapTimeoutMs,
            uint16_t                    flowPulsesPerLitre,
            Runtime &                   runtime,
            Sensor &                    flow,
            Sensor &                    temperature,
            Sensor &                    pressure,
            IRingBuffer< Event > &      eventHistory,
            uint32_t &                  lastSeqNum,
            Delegate< void( Event & ) > onEventEnd = {}
        ) :
            _tapTimeoutMs( tapTimeoutMs ),
            _flowPulsesPerLitre(
                ( flowPulsesPerLitre > 0 ) ? flowPulsesPerLitre : DEFAULT_FLOW_PULSES_PER_LITRE
            ),
            _usedDefaultFlowPulsesPerLitre( flowPulsesPerLitre == 0 ),
            _runtime( runtime ),
            _flow( flow ),
            _temperature( temperature ),
            _pressure( pressure ),
            _eventHistory( eventHistory ),
            _lastSeqNum( lastSeqNum ),
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

            if ( _usedDefaultFlowPulsesPerLitre )
            {
                Log::warn(
                    "flowPulsesPerLitre is 0, using default %u",
                    static_cast< unsigned >( DEFAULT_FLOW_PULSES_PER_LITRE )
                );
            }

            Log::info( "Initialized" );
            return true;
        }

        uint32_t getCurrentPouredMl() const
        {
            if ( ! _eventActive )
            {
                return 0;
            }

            uint64_t volumeAccNow = _volumeAcc;
            if ( _prevFlowSample.timestamp != 0 && _runtime.flow.timestamp >= _prevFlowSample.timestamp )
            {
                uint64_t dt = _runtime.flow.timestamp - _prevFlowSample.timestamp;
                volumeAccNow += static_cast< uint64_t >( _prevFlowSample.value ) * dt;
            }

            uint32_t volumeFromFlowMl = _divideRoundNearest( volumeAccNow, MS_PER_MINUTE );

            uint32_t volumeFromSampleMl = 0;
            if ( _pulseAcc > 0 )
            {
                volumeFromSampleMl = _divideRoundNearest( _pulseAcc * ML_PER_LITRE, _flowPulsesPerLitre );
            }

            return ( volumeFromSampleMl > 0 ) ? volumeFromSampleMl : volumeFromFlowMl;
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

            if ( _runtime.flow.timestamp == 0 )
            {
                return;
            }

            // Control task can run faster than data source polling; ignore repeated samples.
            if ( _runtime.flow.timestamp == _lastProcessedFlowTimestamp )
            {
                return;
            }

            DataSource::Base::Sample prevObservedFlow = _lastObservedFlowSample;
            _lastProcessedFlowTimestamp               = _runtime.flow.timestamp;

            if ( ! _eventActive )
            {
                if ( _runtime.flow.value == 0 )
                {
                    _lastObservedFlowSample = _runtime.flow;
                    return;
                }

                startEvent( _runtime.flow.timestamp );
            }

            if ( _eventActive )
            {
                _addSample( _runtime.flow, _runtime.temperature, _runtime.pressure );

                if ( _runtime.flow.value > 0 )
                {
                    _lastNonZeroFlowSample = _runtime.flow;
                }

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

            _lastObservedFlowSample = _runtime.flow;
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
            if ( ! _eventHistory.empty() && _eventHistory.back().sequenceNumber != _lastSeqNum )
            {
                Log::warn(
                    "Sequence number mismatch: history back=%llu, lastSeqNum=%u — using lastSeqNum",
                    _eventHistory.back().sequenceNumber,
                    _lastSeqNum
                );
            }

            _currentEvent->sequenceNumber = static_cast< uint64_t >( _lastSeqNum ) + 1;
            _currentEvent->startTimestamp = timestamp;

            _volumeAcc                    = 0;
            _volumeFromSampleAccMl        = 0;
            _pulseAcc                     = 0;
            _prevFlowSample               = {};
            _lastNonZeroFlowSample        = {};
            _dsWindowStart                = 0;
            _dsFlowAcc                    = 0;
            _dsTempAcc                    = 0;
            _dsPulseAcc                   = 0;
            _dsCount                      = 0;
            _dsOutIdx                     = 0;

            if ( _runtime.flow.value > 0 )
            {
                _lastNonZeroFlowSample = _runtime.flow;
            }

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

            const uint64_t effectiveEndTimestamp =
                ( _lastNonZeroFlowSample.timestamp > 0 ) ? _lastNonZeroFlowSample.timestamp : timestamp;

            // Finalize volume: add contribution from last sample to endTimestamp
            if ( _prevFlowSample.timestamp != 0 && effectiveEndTimestamp >= _prevFlowSample.timestamp )
            {
                uint64_t dt = effectiveEndTimestamp - _prevFlowSample.timestamp;
                _volumeAcc += static_cast< uint64_t >( _prevFlowSample.value ) * dt;
            }

            // Flush remaining downsample window
            _flushDownsampleWindow();

            uint64_t pulseDelta = _pulseAcc;
            if ( pulseDelta > 0 )
            {
                _volumeFromSampleAccMl = _divideRoundNearest( pulseDelta * ML_PER_LITRE, _flowPulsesPerLitre );
            }

            uint32_t volumeFromFlowMl = _divideRoundNearest( _volumeAcc, MS_PER_MINUTE );
            uint32_t volumeFromSampleMl = static_cast< uint32_t >( _volumeFromSampleAccMl );

            _currentEvent->endTimestamp = effectiveEndTimestamp;
            _currentEvent->volume       = volumeFromSampleMl;
          //  _currentEvent->volume       = volumeFromFlowMl;
            _currentEvent->pulseCount   = pulseDelta;

            if ( _onEventEnd.is_valid() )
            {
                _onEventEnd( *_currentEvent );
            }

            if ( ! _currentEvent->discarded )
            {
                _eventHistory.push( *_currentEvent );
            }
            else
            {
                Log::info(
                    "Discarded event seq=%llu from history push (pulses=%llu, volume=%u)",
                    _currentEvent->sequenceNumber,
                    _currentEvent->pulseCount,
                    _currentEvent->volume
                );
            }
            _lastSeqNum = static_cast< uint32_t >( _currentEvent->sequenceNumber );

            Log::warn(
                "Event: startTimestamp=%llu, endTimestamp=%llu, pulses=%llu, volumeFlow=%u, volumeSample=%u, type=%u, historySlots=%u",
                _currentEvent->startTimestamp,
                _currentEvent->endTimestamp,
                pulseDelta,
                volumeFromFlowMl,
                volumeFromSampleMl,
                static_cast< uint8_t >( _currentEvent->type ),
                static_cast< uint32_t >( _dsOutIdx )
            );

            memset( _currentEvent, 0, sizeof( Event ) );

            Log::info( "Event ended" );
            return true;
        }

    private:
        static constexpr const char TAG[]              = "EventMonitor";
        using Log                                      = Logger< ProjectConfig::LOG_LEVEL_MEASUREMENT, TAG >;

        static constexpr uint32_t MS_PER_MINUTE        = 60'000;
        static constexpr uint32_t DOWNSAMPLE_WINDOW_MS = 200;
        static constexpr uint32_t ML_PER_LITRE         = 1000;
        static constexpr uint16_t DEFAULT_FLOW_PULSES_PER_LITRE = 472;

        static uint32_t _divideRoundNearest( uint64_t numerator, uint32_t denominator )
        {
            if ( denominator == 0 ) return 0;
            return static_cast< uint32_t >( ( numerator + ( denominator / 2 ) ) / denominator );
        }

        uint32_t & _tapTimeoutMs;
        uint16_t   _flowPulsesPerLitre;
        bool       _usedDefaultFlowPulsesPerLitre = false;
        Runtime &  _runtime;

        bool     _eventActive     = false;
        uint64_t _zeroFlowStartMs = 0;

        Sensor & _flow;
        Sensor & _temperature;
        Sensor & _pressure;

        Event * _currentEvent =
            static_cast< Event * >( heap_caps_calloc( 1, sizeof( Event ), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT ) );

        IRingBuffer< Event > &      _eventHistory;
        uint32_t &                  _lastSeqNum;
        Delegate< void( Event & ) > _onEventEnd;

        // MARK: IncrementalVolume
        uint64_t                 _volumeAcc             = 0;
        uint64_t                 _volumeFromSampleAccMl = 0;
        uint64_t                 _pulseAcc              = 0;
        DataSource::Base::Sample _prevFlowSample        = {};
        DataSource::Base::Sample _lastNonZeroFlowSample = {};
        uint64_t                 _lastProcessedFlowTimestamp = 0;
        DataSource::Base::Sample _lastObservedFlowSample = {};

        // MARK: IncrementalDownsampling
        uint64_t _dsWindowStart                  = 0;
        uint64_t _dsFlowAcc                      = 0;
        uint64_t _dsTempAcc                      = 0;
        uint64_t _dsPulseAcc                     = 0;
        uint32_t _dsCount                        = 0;
        uint16_t _dsOutIdx                       = 0;

        uint32_t _currentVolumeFromFlowMl() const
        {
            uint64_t volumeAccNow = _volumeAcc;
            if ( _prevFlowSample.timestamp != 0 && _runtime.flow.timestamp >= _prevFlowSample.timestamp )
            {
                uint64_t dt = _runtime.flow.timestamp - _prevFlowSample.timestamp;
                volumeAccNow += static_cast< uint64_t >( _prevFlowSample.value ) * dt;
            }

            return _divideRoundNearest( volumeAccNow, MS_PER_MINUTE );
        }

        uint32_t _currentVolumeFromPulseMl() const
        {
            if ( _pulseAcc == 0 ) return 0;
            return _divideRoundNearest( _pulseAcc * ML_PER_LITRE, _flowPulsesPerLitre );
        }

        void _addSample(
            const DataSource::Base::Sample & flow,
            const DataSource::Base::Sample & temp,
            const DataSource::Base::Sample & press
        )
        {
            (void) press;

            // Incremental volume: accumulate (prevValue * dt) for each new sample
            if ( _prevFlowSample.timestamp != 0 )
            {
                uint64_t dt = flow.timestamp - _prevFlowSample.timestamp;
                _volumeAcc += static_cast< uint64_t >( _prevFlowSample.value ) * dt;

            }

            _pulseAcc += flow.pulse;

            _prevFlowSample = flow;

            // Incremental downsampling: accumulate into current window
            if ( _dsCount == 0 )
            {
                _dsWindowStart = flow.timestamp;
            }

            _dsFlowAcc  += flow.value;
            _dsTempAcc  += temp.value;
            _dsPulseAcc += flow.pulse;
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

            const uint32_t volumeFromFlowMl  = _currentVolumeFromFlowMl();
            const uint32_t volumeFromPulseMl = _currentVolumeFromPulseMl();

            _currentEvent->flowProfile[ _dsOutIdx ]        = _divideRoundNearest( _dsFlowAcc, _dsCount );
            _currentEvent->volumeFlowProfile[ _dsOutIdx ]  = volumeFromFlowMl;
            _currentEvent->volumePulseProfile[ _dsOutIdx ] = volumeFromPulseMl;
            _currentEvent->pulseProfile[ _dsOutIdx ]       = static_cast< uint32_t >( _pulseAcc );
            ++_dsOutIdx;

            _dsFlowAcc  = 0;
            _dsTempAcc  = 0;
            _dsPulseAcc = 0;
            _dsCount    = 0;
        }
    };
}    // namespace AsnPlus

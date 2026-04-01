#pragma once

#include "program/config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/ring_buffer.hpp"

#include "sensor.hpp"

namespace AsnPlus
{
    class EventMonitor
    {
    public:
        static constexpr uint8_t MAX_EVENT_HISTORY = 100;

        struct Runtime: AsnPlus::Runtime
        {
            uint16_t flow = 0;
            uint16_t temperature = 0;
            uint16_t pressure = 0;
        };

        struct Event
        {
            static constexpr uint8_t HISTORY_SIZE = 32;
            enum class EventType : uint8_t
            {
                UNKNOWN,
                TBD
            };

            uint64_t sequenceNumber = 0;
            uint64_t startTimestamp = 0;
            uint64_t endTimestamp   = 0;
            bool     synced         = false;

            uint16_t flowHistory[ HISTORY_SIZE ] {};
            uint16_t temperatureHistory[ HISTORY_SIZE ] {};
            uint16_t pressureHistory[ HISTORY_SIZE ] {};
        };

        EventMonitor(
            uint32_t &              tapTimeoutMs,
            Runtime &               runtime,
            Sensor &                flow,
            Sensor &                temperature,
            Sensor &                pressure,
            IRingBuffer< Event > &  event_history
        ) :
            _tapTimeoutMs( tapTimeoutMs ),
            _runtime( runtime ),
            _flow( flow ),
            _temperature( temperature ),
            _pressure( pressure ),
            _eventHistory( event_history )
        {
        }

        bool initialize()
        {
            Log::debug( "Initializing" );
            Log::info( "Initialized" );
            return true;
        }

        void poll()
        {
            _runtime.flow = _flow.read();
            _runtime.temperature = _temperature.read();
            _runtime.pressure = _pressure.read();

            if ( ! _eventActive )
            {
                if ( _flow.read() == 0 )
                {
                    Log::debug( "No flow detected, skipping event start" );
                    return;
                }

                _flowHistoryBuffer.clear();
                _temperatureHistoryBuffer.clear();
                _pressureHistoryBuffer.clear();
                startEvent();
            }

            if ( _eventActive )
            {
                _flowHistoryBuffer.push( _runtime.flow );
                _temperatureHistoryBuffer.push( _runtime.temperature );
                _pressureHistoryBuffer.push( _runtime.pressure );

                // TODO (DK): Implement tap timeout logic to end event if flow stops for a certain period of time
            }
        }

        bool startEvent()
        {
            Log::debug( "Starting event" );
            if ( _eventActive )
            {
                Log::warn( "Event is already active" );
                return false;
            }

            _eventActive                 = true;

            _currentEvent                = Event {};
            _currentEvent.sequenceNumber = _eventHistory.empty() ? 1 : _eventHistory.back().sequenceNumber + 1;
            _currentEvent.startTimestamp = Utils::getMs64();

            Log::info( "Event started" );
            return true;
        }

        bool endEvent()
        {
            Log::debug( "Ending event" );

            if ( ! _eventActive )
            {
                Log::warn( "Event is not active" );
                return false;
            }

            _eventActive               = false;

            _currentEvent.endTimestamp = Utils::getMs64();
            _eventHistory.push( _currentEvent );

            _currentEvent = Event {};
            _flowHistoryBuffer.clear();
            _temperatureHistoryBuffer.clear();
            _pressureHistoryBuffer.clear();

            Log::info( "Event ended" );
            return true;
        }

    private:
        static constexpr const char TAG[] = "EventMonitor";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_MEASUREMENT, TAG >;

        uint32_t & _tapTimeoutMs;
        Runtime &  _runtime;

        bool _eventActive = false;

        Sensor &        _flow;
        Sensor & _temperature;
        Sensor &    _pressure;

        Event _currentEvent {};

        RingBufferExt< uint16_t > _flowHistoryBuffer { _currentEvent.flowHistory, Event::HISTORY_SIZE };
        RingBufferExt< uint16_t > _temperatureHistoryBuffer { _currentEvent.temperatureHistory, Event::HISTORY_SIZE };
        RingBufferExt< uint16_t > _pressureHistoryBuffer { _currentEvent.pressureHistory, Event::HISTORY_SIZE };

        IRingBuffer< Event > & _eventHistory;
    };
}    // namespace AsnPlus

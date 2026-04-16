#pragma once

#include "build_info.h"
#include "program/config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/ring_buffer.hpp"
#include "asn/asn-core/vector.hpp"

#include "asn/asn-hal/time_manager/structs.hpp"

#include "event_monitor.hpp"

namespace AsnPlus
{
    /**
     * @brief Monitors system time for a forward jump from an invalid epoch
     *        (below BUILD_TIMESTAMP_MS) into a valid one, then retroactively
     *        corrects the start/end timestamps of all queued events by the
     *        same offset.
     *
     *        Must be polled after TimeManager so that TimeRuntime is already
     *        up to date for the current tick.
     */
    class HistoryManager
    {
    public:
        HistoryManager(
            const TimeRuntime &                                       timeRuntime,
            IVector< IRingBuffer< EventMonitor::Event > * > &         eventHistories
        ) :
            _timeRuntime( timeRuntime ),
            _eventHistories( eventHistories )
        {
        }

        bool initialize()
        {
            Log::debug( "Initializing" );
            _lastKnownTime = _timeRuntime.utcEpochMs;
            Log::info( "Initialized (lastKnownTime=%llu)", _lastKnownTime );
            return true;
        }

        void poll()
        {
            const uint64_t current = _timeRuntime.utcEpochMs;

            Log::debug(
                "Poll: lastKnownTime=%llu, current=%llu, BUILD_TIMESTAMP_MS=%llu",
                _lastKnownTime,
                current,
                static_cast< uint64_t >( BUILD_TIMESTAMP_MS )
            );

            if ( _lastKnownTime < BUILD_TIMESTAMP_MS && current >= BUILD_TIMESTAMP_MS )
            {
                _correctionOffset = current - _lastKnownTime;
                Log::info(
                    "Time corrected: %llu → %llu (offset=%llu ms), will patch history each poll",
                    _lastKnownTime,
                    current,
                    _correctionOffset
                );
            }

            if ( _correctionOffset > 0 ) _patchAllHistories( _correctionOffset );

            _lastKnownTime = current;
        }

    private:
        static constexpr const char TAG[] = "HistoryManager";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_MEASUREMENT, TAG >;

        const TimeRuntime &                                 _timeRuntime;
        IVector< IRingBuffer< EventMonitor::Event > * > &   _eventHistories;

        uint64_t _lastKnownTime    = 0;
        uint64_t _correctionOffset = 0;

        void _patchAllHistories( uint64_t offset )
        {
            uint8_t channel = 0;
            for ( auto * history : _eventHistories )
            {
                _patchHistory( *history, offset, channel );
                ++channel;
            }
        }

        void _patchHistory( IRingBuffer< EventMonitor::Event > & history, uint64_t offset, uint8_t channel )
        {
            Log::info( "Channel %u: %u event(s) in history", channel, static_cast< uint32_t >( history.size() ) );

            uint32_t count = 0;
            for ( auto & event : history )
            {
                const bool fixStart = event.startTimestamp < BUILD_TIMESTAMP_MS;
                const bool fixEnd   = event.endTimestamp < BUILD_TIMESTAMP_MS;

                if ( ! fixStart && ! fixEnd )
                {
                    Log::debug(
                        "Channel %u event #%llu: skipped (both timestamps already valid)",
                        channel,
                        event.sequenceNumber
                    );
                    continue;
                }

                Log::info(
                    "Channel %u event #%llu: start %llu → %llu, end %llu → %llu",
                    channel,
                    event.sequenceNumber,
                    event.startTimestamp,
                    fixStart ? event.startTimestamp + offset : event.startTimestamp,
                    event.endTimestamp,
                    fixEnd ? event.endTimestamp + offset : event.endTimestamp
                );

                if ( fixStart ) event.startTimestamp += offset;
                if ( fixEnd )   event.endTimestamp   += offset;
                ++count;
            }

            Log::info( "Channel %u: patched %u event(s)", channel, count );
        }
    };
}    // namespace AsnPlus

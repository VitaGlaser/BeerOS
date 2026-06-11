#pragma once

#include "program/config.hpp"

#include "asn/asn-core/logger.hpp"

#include "asn/asn-hal/include/time_manager/time_manager.hpp"

#include "asn/asn-expander-lib/include/timer/timer.hpp"

#include "esp_rom_sys.h"

#include "base.hpp"

namespace AsnPlus::DataSource
{
    class FlowPulse : public Base
    {
    public:
        FlowPulse( uint16_t pulsesPerLitre ) : Base(), _pulsesPerLitre( pulsesPerLitre ) {}

        bool initialize() override
        {
            Log::info( "Initialized (pulses/L: %u)", _pulsesPerLitre );
            return true;
        }

        void poll() override
        {
            if ( _channel == nullptr )
            {
                _clearSample();
                return;
            }

            uint16_t count = _channel->getValue();
            // Local retry guard: library timer backend may be precompiled, so recover here too.
            if ( ( count >> 8 ) == ( count & 0xFF ) && count != 0 )
            {
                for ( uint8_t attempt = 0; attempt < SPI_GLITCH_RETRY_COUNT; ++attempt )
                {
                    if ( SPI_GLITCH_RETRY_DELAY_US > 0 )
                    {
                        esp_rom_delay_us( SPI_GLITCH_RETRY_DELAY_US );
                    }

                    uint16_t retry = _channel->getValue();
                    if ( ( ( retry >> 8 ) != ( retry & 0xFF ) ) || retry == 0 )
                    {
                        Log::debug(
                            "Recovered repeated-byte SPI read in poll: bad=0x%04X retry=0x%04X attempt=%u",
                            count,
                            retry,
                            static_cast< unsigned >( attempt + 1 )
                        );
                        count = retry;
                        break;
                    }
                }
            }
            uint64_t timestamp = TimeManager::instance().getUtcTime().toEpochMillis();

            // Last-resort guard: repeated-byte value survived all local retries.
            // Do not rebase baseline on a likely transport glitch, just ignore this sample.
            if ( ( count >> 8 ) == ( count & 0xFF ) && count != 0 )
            {
                _consecutiveSpiGlitches++;
                if ( _consecutiveSpiGlitches == 1 ||
                     ( _consecutiveSpiGlitches % SPI_GLITCH_WARN_EVERY_N ) == 0 )
                {
                    Log::warn(
                        "SPI glitch sample ignored #%u: count=0x%04X (repeated byte 0x%02X).",
                        _consecutiveSpiGlitches,
                        count,
                        count & 0xFF
                    );
                }

                _debugInfo.data[ 0 ] = static_cast< uint8_t >( ( count >> 8 ) & 0xFF );
                _debugInfo.data[ 1 ] = static_cast< uint8_t >( count & 0xFF );
                return;
            }

            if ( _consecutiveSpiGlitches > 0 )
            {
                Log::debug( "SPI glitch streak ended after %u ignored sample(s)", _consecutiveSpiGlitches );
                _consecutiveSpiGlitches = 0;
            }

            // First sample after bind is baseline only; avoid converting stale counter offset to pulses.
            if ( _baselinePending || _lastTimestamp == 0 )
            {
                Log::debug(
                    "Baseline accepted: count=0x%04X (%u), consecutiveGlitches=%u",
                    count,
                    count,
                    _consecutiveGlitches
                );
                _consecutiveGlitches = 0;
                _lastCount           = count;
                _lastTimestamp       = timestamp;
                _baselinePending     = false;

                _debugInfo.data[ 0 ] = static_cast< uint8_t >( ( count >> 8 ) & 0xFF );
                _debugInfo.data[ 1 ] = static_cast< uint8_t >( count & 0xFF );

                _writeSample( timestamp, 0, 0 );
                return;
            }

            uint64_t deltaTimeMs = timestamp - _lastTimestamp;
            if ( deltaTimeMs == 0 )
            {
                return;
            }

            uint16_t deltaPulses =
                ( count >= _lastCount ) ? ( count - _lastCount ) : ( ( 0xFFFF - _lastCount ) + count + 1 );

            // Guard against counter discontinuities (SPI glitch/re-sync/restart).
            // Keep hardware counters untouched; rebase software baseline only.
            uint64_t maxAllowedDeltaByRate =
                ( static_cast< uint64_t >( _pulsesPerLitre ) * MAX_FLOW_ML_PER_MIN * deltaTimeMs ) /
                ( static_cast< uint64_t >( MILLIS_PER_MIN ) * static_cast< uint64_t >( ML_PER_LITRE ) );
            if ( maxAllowedDeltaByRate < 1 ) maxAllowedDeltaByRate = 1;
            maxAllowedDeltaByRate *= RATE_GUARD_MULTIPLIER;

            if ( static_cast< uint64_t >( deltaPulses ) > maxAllowedDeltaByRate )
            {
                _consecutiveGlitches++;
                Log::warn(
                    "Pulse counter discontinuity #%u: delta=%u, dt=%llu ms, count=0x%04X (%u), last=0x%04X (%u), maxAllowed=%llu. Rebased.",
                    _consecutiveGlitches,
                    deltaPulses,
                    deltaTimeMs,
                    count,
                    count,
                    _lastCount,
                    _lastCount,
                    maxAllowedDeltaByRate
                );

                // Force one clean baseline cycle after a discontinuity so a single glitch
                // does not produce two consecutive warning samples.
                _baselinePending = true;
                _lastCount       = 0;
                _lastTimestamp   = 0;

                _debugInfo.data[ 0 ] = static_cast< uint8_t >( ( count >> 8 ) & 0xFF );
                _debugInfo.data[ 1 ] = static_cast< uint8_t >( count & 0xFF );

                _writeSample( timestamp, 0, 0 );
                return;
            }

            uint32_t value = 0;
            if ( _pulsesPerLitre > 0 )
            {
                value = static_cast< uint32_t >(
                    static_cast< float >( deltaPulses * ML_PER_LITRE * MILLIS_PER_MIN ) /
                    ( static_cast< float >( _pulsesPerLitre * deltaTimeMs ) )
                );
            }

            _lastCount     = count;
            _lastTimestamp = timestamp;

            // Store pulse count in debug info for visibility;
            _debugInfo.data[ 0 ] = static_cast< uint8_t >( ( count >> 8 ) & 0xFF );
            _debugInfo.data[ 1 ] = static_cast< uint8_t >( count & 0xFF );

            _writeSample( timestamp, value, deltaPulses );

            Log::debug( "Polled pulse flow sensor: flow=%u ml/min, pulsesDelta=%u (%llu ms)", value, deltaPulses, timestamp );
        }

        void bindTimerChannel( Expander::Timers::Timer::Channel & channel )
        {
            _baselinePending = true;
            _lastTimestamp   = 0;
            _lastCount       = 0;
            _channel = &channel;
        }

        void unbindTimerChannel()
        {
            _channel         = nullptr;
            _baselinePending = true;
            _lastTimestamp   = 0;
            _lastCount       = 0;
        }

    private:
        static constexpr const char TAG[]           = "FlowPulseDataSource";
        using Log                                   = Logger< ProjectConfig::LOG_LEVEL_DATA_SOURCES, TAG >;

        static constexpr float ML_PER_LITRE            = 1000.0f;
        static constexpr uint32_t MAX_FLOW_ML_PER_MIN  = 12000;
        static constexpr uint8_t RATE_GUARD_MULTIPLIER = 4;
        static constexpr uint8_t SPI_GLITCH_RETRY_COUNT = 3;
        static constexpr uint16_t SPI_GLITCH_RETRY_DELAY_US = 100;
        static constexpr uint16_t SPI_GLITCH_WARN_EVERY_N = 32;

        Expander::Timers::Timer::Channel * _channel = nullptr;
        uint16_t                           _pulsesPerLitre;

        uint16_t _lastCount           = 0;
        uint64_t _lastTimestamp       = 0;
        bool     _baselinePending     = true;
        uint32_t _consecutiveGlitches = 0;
        uint32_t _consecutiveSpiGlitches = 0;
    };
}    // namespace AsnPlus::DataSource

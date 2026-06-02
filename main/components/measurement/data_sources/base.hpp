#pragma once

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/mutex.hpp"
#include "asn/asn-core/types.hpp"

namespace AsnPlus::DataSource
{
    class Base
    {
    public:
        struct Sample
        {
            uint64_t timestamp = 0;
            uint32_t value     = 0;
            uint64_t volume    = 0;
            uint16_t pulsesPerLitre = 0;
        };

        struct DebugInfo
        {
            static constexpr uint8_t INFO_SIZE = 8;
            uint8_t                  data[ INFO_SIZE ] = {};
        };

        Base()                    = default;

        virtual bool initialize() = 0;

        virtual void poll()       = 0;

        Sample read() const
        {
            LockGuard guard( _sampleMux );
            return _sample;
        }

        uint64_t getId() const { return _id; }

        void setId( uint64_t id ) { _id = id; }

        void enable() { _enabled = true; }

        void disable() { _enabled = false; }

        bool isEnabled() const { return _enabled; }

        DebugInfo getDebugInfo() const
        {
            LockGuard guard( _sampleMux );
            return _debugInfo;
        }

    protected:
        bool     _enabled = false;
        uint64_t _id      = 0;
        Sample   _sample  = {};

        DebugInfo _debugInfo = {};

        mutable Mutex _sampleMux {};

        void _writeSample( uint64_t timestamp, uint32_t value, uint64_t volume = 0, uint16_t pulsesPerLitre = 0 )
        {
            LockGuard guard( _sampleMux );
            _sample.timestamp      = timestamp;
            _sample.value          = value;
            _sample.volume         = volume;
            _sample.pulsesPerLitre = pulsesPerLitre;
        }

        void _clearSample()
        {
            LockGuard guard( _sampleMux );
            _sample = {};
        }
    };
}    // namespace AsnPlus::DataSource

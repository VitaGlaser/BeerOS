#pragma once

#include <cstdint>

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/timer.hpp"
#include "asn/asn-hal/include/peripherals/persistent_storage.hpp"
#include "asn/asn-hal/include/time_manager/rtc.hpp"
#include "asn/asn-hal/include/time_manager/structs.hpp"

namespace AsnPlus
{
    class ApplicationTimeManager
    {
    public:
        ApplicationTimeManager(
            ISystemClock &       systemClock,
            IRtc *               rtc,
            IPersistentStorage & storage,
            TimeConfig &         timeConfig,
            TimeRuntime &        timeRuntime,
            TimeChangeRequest &  timeChangeRequest
        );

        static ApplicationTimeManager & instance() { return *_instance; }

        const TimeRuntime & getRuntime() const { return _timeRuntime; }

        Time getLocalTime() const { return _systemClock.getLocalTime(); }

        Time getUtcTime() const { return _systemClock.getUtc(); }

        bool initialize();
        void poll();

        void onNetworkTimeSync( uint64_t epochMs );

    private:
        static constexpr const char TAG[]                 = "ApplicationTimeManager";
        using Log                                         = Logger< 2, TAG >;

        static constexpr const char TIME_CONFIG_NVS_KEY[] = "time_cfg";
        static inline ApplicationTimeManager * _instance  = nullptr;

        ISystemClock &       _systemClock;
        IRtc *               _rtc;
        IPersistentStorage & _storage;

        uint64_t _lastTimeConfigTimestamp = 0;

        TimeConfig &        _timeConfig;
        TimeRuntime &       _timeRuntime;
        TimeChangeRequest & _timeChangeRequest;

        bool _initialized = false;

        void _loadTimeConfig();
        void _saveTimeConfig();
        void _syncTimeFromRtc();
        void _applyTimeChangeRequest();
        void _applyTimeConfig();
        void _setSystemAndRtc( const Time & utc );
        void _updateRuntime();
    };
}    // namespace AsnPlus
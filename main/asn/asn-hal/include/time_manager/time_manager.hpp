#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/timer.hpp"

#include "peripherals/persistent_storage.hpp"

#include "rtc.hpp"
#include "structs.hpp"

namespace AsnPlus
{
    // TODO (DK): Test this class when there's an update in the app so that the app uses the correct service and
    // characteristics. The main points of testing should be the TimeConfig interaction with NVS, setting the time to
    // correct value via Factory app and setting different timezones and having different local time.
    class TimeManager
    {
    public:
        TimeManager(
            ISystemClock &       systemClock,
            IRtc *               rtc,
            IPersistentStorage & storage,
            TimeConfig &         timeConfig,
            TimeRuntime &        timeRuntime,
            TimeChangeRequest &  timeChangeRequest
        );

        static TimeManager & instance() { return *_instance; }

        const TimeRuntime & getRuntime() const { return _timeRuntime; }

        Time getLocalTime() const { return _systemClock.getLocalTime(); }

        Time getUtcTime() const { return _systemClock.getUtc(); }

        bool initialize();
        void poll();

    private:
        static constexpr const char TAG[]                 = "TimeManager";
        using Log                                         = Logger< ModuleConfig::Time::LOG_LEVEL, TAG >;

        static constexpr const char TIME_CONFIG_NVS_KEY[] = "time_cfg";
        static constexpr uint32_t   RTC_SYNC_INTERVAL     = 60 * 1000;

        static inline TimeManager * _instance             = nullptr;

        ISystemClock &       _systemClock;
        IRtc *               _rtc;
        IPersistentStorage & _storage;

        uint64_t _lastTimeConfigTimestamp = 0;

        TimeConfig &        _timeConfig;
        TimeRuntime &       _timeRuntime;
        TimeChangeRequest & _timeChangeRequest;

        Timer<> _rtcSyncTimer {};

        void _syncTimeFromRtc();
        void _setNewTime( TimeChangeRequest timeRequest );
        void _updateRtcTime( Time time );
        void _setNewTimezone( const char * timezone );
        void _saveTimeConfig();
        void _loadTimeConfig();
        void _updateRuntime();
    };
}    // namespace AsnPlus

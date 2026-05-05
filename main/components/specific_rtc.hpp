#pragma once

#include "asn/asn-drivers/pcf85263.hpp"

#include "asn/asn-hal/include/time_manager/rtc.hpp"

namespace AsnPlus
{
    class SpecificRtc : public IRtc
    {
    public:
        explicit SpecificRtc( Drivers::PCF85263A & rtc ) : _rtc( rtc ) {}

        bool initialize() override
        {
            _rtc.initialize();
            return true;
        }

        void poll() override {}

        void setUtc( const Time & time ) override
        {
            if ( ! time.isValid() ) return;

            const time_t epochSec = static_cast< time_t >( time.toEpochMillis() / 1000ULL );

            tm timeinfo {};
            gmtime_r( &epochSec, &timeinfo );

            _rtc.setTime(
                timeinfo.tm_sec,
                timeinfo.tm_min,
                timeinfo.tm_hour,
                timeinfo.tm_wday,       // 0=Sun..6=Sat
                timeinfo.tm_mday,
                timeinfo.tm_mon + 1,    // 1..12
                timeinfo.tm_year + 1900
            );
        }

        Time getUtc() override
        {
            _rtc.updateTime();

            Time time = {
                .year   = _rtc.getYear(),
                .month  = _rtc.getMonth(),
                .day    = _rtc.getDate(),
                .hour   = _rtc.getHours(),
                .minute = _rtc.getMinutes(),
                .second = _rtc.getSeconds()
            };
            return time;
        }

    private:
        static constexpr const char TAG[] = "SpecificRtc";
        using Log                         = Logger< ModuleConfig::Drivers::LOG_LEVEL, TAG >;

        Drivers::PCF85263A & _rtc;
    };
}    // namespace AsnPlus

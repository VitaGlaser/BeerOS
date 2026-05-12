#pragma once

#include "asn_module_config.hpp"

#include <cstdlib>
#include <cstring>
#include <sys/time.h>
#include <time.h>

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/time.hpp"
#include "asn/asn-core/types.hpp"

#include "asn/asn-hal/include/time_manager/rtc.hpp"

namespace AsnPlus::Esp32
{
    class SystemClock : public ISystemClock
    {
    public:
        bool initialize() override;
        void poll() override;
        void setUtc( const Time & utc ) override;
        Time getUtc() override;
        void setLocalTime( const Time & local ) override;
        Time getLocalTime() override;
        void setTimezone( const char * timezone ) override;
        const char * getTimezone() override;

    private:
        static constexpr const char TAG[] = "SystemClock";
        using Log                         = Logger< ModuleConfig::Esp32::LOG_LEVEL, TAG >;

        String< 32 > _timezone            = "UTC0";

        void _setTimezoneInternal( const char * tz );
        static bool _localToEpochMs( const Time & local, uint64_t & outEpochMs );
    };
}    // namespace AsnPlus::Esp32

#pragma once

#include "asn/asn-core/time.hpp"
#include "asn/asn-core/types.hpp"

namespace AsnPlus
{
    /**
     * Interface for RTC
     * ! this should work only with UTC !
     */
    class IRtc
    {
    public:
        virtual bool initialize()                = 0;
        virtual void poll()                      = 0;

        virtual void reset()                     = 0;
        virtual bool hasValidTime()              = 0;

        virtual void setUtc( const Time & time ) = 0;
        virtual Time getUtc()                    = 0;
    };

    /**
     * Interface for System time handling
     * ! this is the one working with timezones !
     */
    class ISystemClock : public IRtc
    {
    public:
        virtual void setLocalTime( const Time & time )            = 0;
        virtual Time getLocalTime()                               = 0;

        virtual void         setTimezone( const char * timezone ) = 0;
        virtual const char * getTimezone()                        = 0;
    };
}    // namespace AsnPlus

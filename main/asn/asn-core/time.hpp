#pragma once

#include "string.hpp"
#include "types.hpp"

// TODO(DK): Use this as wrapper only ad swap the logic for ctime/chrono

namespace AsnPlus
{
    static constexpr uint32_t MICROS_PER_MILI = 1000u;
    static constexpr uint32_t MICROS_PER_SEC  = 1000u * MICROS_PER_MILI;
    static constexpr uint32_t MICROS_PER_MIN  = 60u * MICROS_PER_SEC;
    static constexpr uint32_t MICROS_PER_HOUR = 60u * MICROS_PER_MIN;
    static constexpr uint32_t MICROS_PER_DAY  = 24u * MICROS_PER_HOUR;

    static constexpr uint32_t MILLIS_PER_SEC  = 1000u;
    static constexpr uint32_t MILLIS_PER_MIN  = 60u * MILLIS_PER_SEC;
    static constexpr uint32_t MILLIS_PER_HOUR = 60u * MILLIS_PER_MIN;
    static constexpr uint32_t MILLIS_PER_DAY  = 24u * MILLIS_PER_HOUR;

    static constexpr uint32_t SECS_PER_MIN    = 60u;
    static constexpr uint32_t SECS_PER_HOUR   = 60u * SECS_PER_MIN;
    static constexpr uint32_t SECS_PER_DAY    = 24u * SECS_PER_HOUR;

    struct Time
    {
        static constexpr uint8_t MONTH_BASE[ 12 ]            = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        static constexpr uint8_t WEEKDAY_MONTH_OFFSETS[ 12 ] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };

        uint16_t year                                        = 1970;
        uint8_t  month                                       = 1;    // 1..12
        uint8_t  weekday                                     = 0;    // 0..6 (Sun..Sat) for PCF85063A-style mapping
        uint8_t  day                                         = 1;    // 1..31
        uint8_t  hour                                        = 0;    // 0..23
        uint8_t  minute                                      = 0;    // 0..59
        uint8_t  second                                      = 0;    // 0..59
        uint16_t millisecond                                 = 0;    // 0..999

        static constexpr bool isLeapYear( uint16_t y )
        {
            return ( ( y % 4 ) == 0 ) && ( ( y % 100 ) != 0 || ( y % 400 ) == 0 );
        }

        static constexpr uint8_t daysInMonth( uint16_t y, uint8_t m )
        {
            if ( m < 1 || m > 12 ) return 0;
            if ( m != 2 ) return MONTH_BASE[ m - 1 ];
            return static_cast< uint8_t >( isLeapYear( y ) ? 29 : 28 );
        }

        static constexpr uint8_t weekdayFromYmd( uint16_t y, uint8_t m, uint8_t d )
        {
            // Valid for Gregorian calendar; typical embedded RTC range is fine.
            // Sakamoto algorithm: returns 0=Sunday .. 6=Saturday.
            if ( m < 3 )
            {
                y = static_cast< uint16_t >( y - 1 );
            }

            return static_cast< uint8_t >(
                ( y + y / 4u - y / 100u + y / 400u + WEEKDAY_MONTH_OFFSETS[ m - 1u ] + d ) % 7u
            );
        }

        void recalcWeekday()
        {
            // Only update if date is valid enough to compute.
            if ( isValid() )
            {
                weekday = weekdayFromYmd( year, month, day );
            }
        }

        bool isValid() const
        {
            if ( year < 1970 || year > 9999 ) return false;
            if ( month < 1 || month > 12 ) return false;
            if ( weekday > 6 ) return false;
            const uint8_t dim = daysInMonth( year, month );
            if ( dim == 0 ) return false;
            if ( day < 1 || day > dim ) return false;
            if ( hour > 23 ) return false;
            if ( minute > 59 ) return false;
            if ( second > 59 ) return false;
            if ( millisecond > 999 ) return false;
            return true;
        }

        // UTC only, ignores leap seconds (like Unix time).
        // Returns 0 on invalid input.
        uint64_t toEpochMillis() const
        {
            if ( ! isValid() ) return 0;

            uint64_t days = 0;

            for ( uint16_t y = 1970; y < year; ++y )
            {
                days += isLeapYear( y ) ? 366u : 365u;
            }

            for ( uint8_t m = 1; m < month; ++m )
            {
                days += daysInMonth( year, m );
            }

            days                  += static_cast< uint64_t >( day - 1 );

            const uint64_t msOfDay = static_cast< uint64_t >( hour ) * MILLIS_PER_HOUR +
                static_cast< uint64_t >( minute ) * MILLIS_PER_MIN +
                static_cast< uint64_t >( second ) * MILLIS_PER_SEC + static_cast< uint64_t >( millisecond );

            return days * MILLIS_PER_DAY + msOfDay;
        }

        // UTC only, ignores leap seconds (like Unix time).
        void fromEpochMillis( uint64_t epochMillis )
        {
            uint64_t days    = epochMillis / MILLIS_PER_DAY;
            uint32_t msOfDay = static_cast< uint32_t >( epochMillis % MILLIS_PER_DAY );

            hour             = static_cast< uint8_t >( msOfDay / MILLIS_PER_HOUR );
            msOfDay         %= MILLIS_PER_HOUR;

            minute           = static_cast< uint8_t >( msOfDay / MILLIS_PER_MIN );
            msOfDay         %= MILLIS_PER_MIN;

            second           = static_cast< uint8_t >( msOfDay / MILLIS_PER_SEC );
            millisecond      = static_cast< uint16_t >( msOfDay % MILLIS_PER_SEC );

            uint16_t y       = 1970;
            while ( true )
            {
                const uint16_t yd = isLeapYear( y ) ? 366u : 365u;
                if ( days < yd ) break;
                days -= yd;
                ++y;
                if ( y == 10'000 ) break;
            }
            year      = y;

            uint8_t m = 1;
            while ( m <= 12 )
            {
                const uint8_t md = daysInMonth( year, m );
                if ( days < md ) break;
                days -= md;
                ++m;
            }
            month   = ( m >= 1 && m <= 12 ) ? m : 12;

            day     = static_cast< uint8_t >( days + 1 );

            weekday = weekdayFromYmd( year, month, day );
        }

        uint32_t getSecondsSinceMidnight() const
        {
            return static_cast< uint32_t >( hour ) * SECS_PER_HOUR +
                static_cast< uint32_t >( minute ) * SECS_PER_MIN +
                static_cast< uint32_t >( second );
        }

        void getReadableString( IString & buffer ) const
        {
            char tmp[ 32 ];

            const int len = snprintf(
                tmp,
                sizeof( tmp ),
                "%04u-%02u-%02u %02u:%02u:%02u.%03u",
                static_cast< unsigned >( year ),
                static_cast< unsigned >( month ),
                static_cast< unsigned >( day ),
                static_cast< unsigned >( hour ),
                static_cast< unsigned >( minute ),
                static_cast< unsigned >( second ),
                static_cast< unsigned >( millisecond )
            );

            buffer.clear();
            if ( len > 0 ) buffer.append( tmp );
        }
    };
}    // namespace AsnPlus

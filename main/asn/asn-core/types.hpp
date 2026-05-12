#pragma once

#include <chrono>
#include <cstdint>

namespace AsnPlus
{
    using u8    = uint8_t;
    using u16   = uint16_t;
    using u32   = uint32_t;
    using u64   = uint64_t;

    using i8    = int8_t;
    using i16   = int16_t;
    using i32   = int32_t;
    using i64   = int64_t;

    using reg8  = volatile u8;
    using reg16 = volatile u16;
    using reg32 = volatile u32;

    using namespace std::chrono_literals;
    using std::chrono::duration_cast;

    using nanoseconds  = std::chrono::duration< u32, std::nano >;
    using microseconds = std::chrono::duration< u32, std::micro >;
    using milliseconds = std::chrono::duration< u32, std::milli >;
    using seconds      = std::chrono::duration< u32 >;
    using minutes      = std::chrono::duration< u32, std::ratio< 60 > >;
    using hours        = std::chrono::duration< u32, std::ratio< 3600 > >;

    struct DateTime
    {
        u8 second, minute, hour, day, month, year;
    };

    struct TimeStamp32
    {
        u32 value {};

        TimeStamp32( DateTime time ) : value { from_date_time( time ) } {}

        static u32 from_date_time( DateTime time )
        {
            return ( ( ( ( time.year * 12 + time.month ) * 31 + time.day ) * 24 + time.hour ) * 60 + time.minute ) *
                60 +
                time.second;
        }

        DateTime to_date_time()
        {
            return {
                .second = (u8) ( value / 1 % 60 ),
                .minute = (u8) ( value / 60 % 60 ),
                .hour   = (u8) ( value / 3600 % 24 ),
                .day    = (u8) ( value / 86'400 % 31 ),
                .month  = (u8) ( value / 2'678'400 % 12 ),
                .year   = (u8) ( value / 32'140'800 ),
            };
        }

        operator u32() { return value; }
    };
}    // namespace AsnPlus

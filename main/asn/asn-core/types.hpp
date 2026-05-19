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


    using std::chrono::duration_cast;

    template< typename T >
    using NSec      = std::chrono::duration< T, std::nano >;

    template< typename T >
    using USec      = std::chrono::duration< T, std::micro >;

    template< typename T >
    using MSec      = std::chrono::duration< T, std::milli >;

    template< typename T >
    using Sec       = std::chrono::duration< T >;

    template< typename T >
    using Min       = std::chrono::duration< T, std::ratio< 60 > >;

    template< typename T >
    using Hours     = std::chrono::duration< T, std::ratio< 3600 > >;


    using nsec32    = NSec< u32 >;
    using usec32    = USec< u32 >;
    using msec32    = MSec< u32 >;
    using sec32     = Sec< u32 >;
    using min32     = Min< u32 >;
    using hours32   = Hours< u32 >;


    inline constexpr
    auto operator ""_ns( u64 value )    { return nsec32( value ); }
    
    inline constexpr
    auto operator ""_us( u64 value )    { return usec32( value ); }

    inline constexpr
    auto operator ""_ms( u64 value )    { return msec32( value ); }

    inline constexpr
    auto operator ""_s( u64 value )     { return sec32( value ); }

    inline constexpr
    auto operator ""_min( u64 value )   { return min32( value ); }

    inline constexpr
    auto operator ""_hr( u64 value )    { return hours32( value ); }


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

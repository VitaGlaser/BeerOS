#pragma once

#include <cstdarg>
#include <cstdio>
#include <ctime>

#include "terminal_codes.hpp"
#include "utils.hpp"

namespace AsnPlus
{
    template< uint8_t LOG_LEVEL, const char * const TAG >
    class Logger
    {
        static void _printDateTime()
        {
            time_t    now = time( nullptr );
            struct tm localTime {};
            localtime_r( &now, &localTime );

            if ( localTime.tm_year < 100 )
            {
                printf( "[time-unsynced]" );
                return;
            }

            printf(
                "[%04d-%02d-%02d %02d:%02d:%02d]",
                localTime.tm_year + 1900,
                localTime.tm_mon + 1,
                localTime.tm_mday,
                localTime.tm_hour,
                localTime.tm_min,
                localTime.tm_sec
            );
        }

        static void _prefix( const char * level, const char * color )
        {
            printf( "%s[%s] (%ld) ", color, level, Utils::getMs() );
            _printDateTime();
            printf( " %s: ", TAG );
        }

    public:
        static constexpr const uint8_t ASN_LOG_LEVEL_DEBUG = 0;
        static constexpr const uint8_t ASN_LOG_LEVEL_INFO  = 1;
        static constexpr const uint8_t ASN_LOG_LEVEL_WARN  = 2;
        static constexpr const uint8_t ASN_LOG_LEVEL_ERROR = 3;
        static constexpr const uint8_t ASN_LOG_LEVEL_NONE  = 4;

        static void temp( const char * format, ... )
        {
            va_list args;
            va_start( args, format );
            _prefix( "TEMP", AnsiCodes::Color::BLACK );
            vprintf( format, args );
            printf( "%s\n", AnsiCodes::Color::RESET );
            va_end( args );
        }

        static void debug( const char * format, ... )
        {
            if ( LOG_LEVEL > ASN_LOG_LEVEL_DEBUG ) return;
            va_list args;
            va_start( args, format );
            _prefix( "DEBUG", AnsiCodes::Color::MAGENTA );
            vprintf( format, args );
            printf( "%s\n", AnsiCodes::Color::RESET );
            va_end( args );
        }

        static void info( const char * format, ... )
        {
            if ( LOG_LEVEL > ASN_LOG_LEVEL_INFO ) return;
            va_list args;
            va_start( args, format );
            _prefix( "INFO", AnsiCodes::Color::GREEN );
            vprintf( format, args );
            printf( "%s\n", AnsiCodes::Color::RESET );
            va_end( args );
        }

        static void warn( const char * format, ... )
        {
            if ( LOG_LEVEL > ASN_LOG_LEVEL_WARN ) return;
            va_list args;
            va_start( args, format );
            _prefix( "WARN", AnsiCodes::Color::YELLOW );
            vprintf( format, args );
            printf( "%s\n", AnsiCodes::Color::RESET );
            va_end( args );
        }

        static void error( const char * format, ... )
        {
            if ( LOG_LEVEL > ASN_LOG_LEVEL_ERROR ) return;
            va_list args;
            va_start( args, format );
            _prefix( "ERROR", AnsiCodes::Color::RED );
            vprintf( format, args );
            printf( "%s\n", AnsiCodes::Color::RESET );
            va_end( args );
        }

        static void hexdump( const char * label, const uint8_t * data, uint32_t len )
        {
            if ( LOG_LEVEL > ASN_LOG_LEVEL_DEBUG ) return;
            _prefix( "DEBUG", AnsiCodes::Color::MAGENTA );
            printf( "%s (%lu bytes):%s\n", label, static_cast< unsigned long >( len ), AnsiCodes::Color::RESET );
            for ( uint32_t i = 0; i < len; i += 32 )
            {
                printf( "  %04lx:  ", static_cast< unsigned long >( i ) );
                for ( uint32_t j = 0; j < 32; ++j )
                {
                    if ( i + j < len ) printf( "%02x ", data[ i + j ] );
                    else printf( "   " );
                    if ( j == 15 ) printf( " " );
                }
                printf( " |" );
                for ( uint32_t j = 0; j < 32 && i + j < len; ++j )
                {
                    uint8_t c = data[ i + j ];
                    printf( "%c", ( c >= 0x20 && c < 0x7f ) ? static_cast< char >( c ) : '.' );
                }
                printf( "|\n" );
            }
        }
    };
}    // namespace AsnPlus
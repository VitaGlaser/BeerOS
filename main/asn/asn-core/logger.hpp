#pragma once

#include <cstdarg>
#include <cstdio>

#include "terminal_codes.hpp"
#include "utils.hpp"

namespace AsnPlus
{
    template< uint8_t LOG_LEVEL, const char * const TAG >
    class Logger
    {
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
            printf( "%s%s[TEMP] (%ld) %s: ", AnsiCodes::Background::WHITE, AnsiCodes::Color::BLACK, Utils::getMs(), TAG );
            vprintf( format, args );
            printf( "%s\n", AnsiCodes::Color::RESET );
            va_end( args );
        }

        static void debug( const char * format, ... )
        {
            if ( LOG_LEVEL > ASN_LOG_LEVEL_DEBUG ) return;

            va_list args;
            va_start( args, format );
            printf( "%s[DEBUG] (%ld) %s: ", AnsiCodes::Color::MAGENTA, Utils::getMs(), TAG );
            vprintf( format, args );
            printf( "%s\n", AnsiCodes::Color::RESET );
            va_end( args );
        }

        static void info( const char * format, ... )
        {
            if ( LOG_LEVEL > ASN_LOG_LEVEL_INFO ) return;

            va_list args;
            va_start( args, format );
            printf( "%s[INFO] (%ld) %s: ", AnsiCodes::Color::GREEN, Utils::getMs(), TAG );
            vprintf( format, args );
            printf( "%s\n", AnsiCodes::Color::RESET );
            va_end( args );
        }

        static void warn( const char * format, ... )
        {
            if ( LOG_LEVEL > ASN_LOG_LEVEL_WARN ) return;

            va_list args;
            va_start( args, format );
            printf( "%s[WARN] (%ld) %s: ", AnsiCodes::Color::YELLOW, Utils::getMs(), TAG );
            vprintf( format, args );
            printf( "%s\n", AnsiCodes::Color::RESET );
            va_end( args );
        }

        static void error( const char * format, ... )
        {
            if ( LOG_LEVEL > ASN_LOG_LEVEL_ERROR ) return;

            va_list args;
            va_start( args, format );
            printf( "%s[ERROR] (%ld) %s: ", AnsiCodes::Color::RED, Utils::getMs(), TAG );
            vprintf( format, args );
            printf( "%s\n", AnsiCodes::Color::RESET );
            va_end( args );
        }

        static void hexdump( const char * label, const uint8_t * data, uint32_t len )
        {
            if ( LOG_LEVEL > ASN_LOG_LEVEL_DEBUG ) return;

            printf( "%s[DEBUG] (%ld) %s: %s (%lu bytes):%s\n", AnsiCodes::Color::MAGENTA, Utils::getMs(), TAG, label, static_cast< unsigned long >( len ),  AnsiCodes::Color::RESET );
            for ( uint32_t i = 0; i < len; i += 32 )
            {
                printf( "  %04lx:  ", static_cast< unsigned long >( i ) );
                for ( uint32_t j = 0; j < 32; ++j )
                {
                    if ( i + j < len )
                        printf( "%02x ", data[ i + j ] );
                    else
                        printf( "   " );
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
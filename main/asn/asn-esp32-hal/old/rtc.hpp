#ifndef _ASNPLUS_ESP32_RTC_HPP
#define _ASNPLUS_ESP32_RTC_HPP

#include "asn/asn-core/timer.hpp"
#include "asn/asn-core/types.hpp"
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

// TODO: Use log.hpp
namespace AsnPlus::Esp32
{
    class Rtc
    {
    public:
        static constexpr const char * TAG = "RTC";

        Timer _timer { Delegate< uint32_t() >::create<
            []() -> uint32_t { return static_cast< uint32_t >( esp_timer_get_time() / 1000 ); } >() };

        void initialize()
        {

            setenv( "TZ", _timezone, 1 );
            tzset();

            initialize_sntp();
        }

        void poll()
        {
            if ( ! _timer.isElapsed() ) return;

            time( &_now );
            localtime_r( &_now, &_timeinfo );

            // ESP_LOGI( TAG, "The current LOC date/time is: %s", asctime( &_timeinfo ) );
            struct tm utc_timeinfo;
            gmtime_r( &_now, &utc_timeinfo );
            // ESP_LOGI( TAG, "The current UTC date/time is: %s", asctime( &utc_timeinfo ) );

            _timer.start( UPDATED_INTERVAL );
        }

        void set_system_time( u32 sec, u32 us )
        {
            xSemaphoreTake( _timeMtx, portMAX_DELAY );
            sntp_set_system_time( sec, us );
            xSemaphoreGive( _timeMtx );
        }

        bool is_time_synchronized() { return sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED; }

        static u64 get_time_milis()
        {
            struct timeval tv;
            gettimeofday( &tv, NULL );
            return ( tv.tv_sec * 1000LL + ( tv.tv_usec / 1000LL ) );
        }

        static u64 get_time_seconds()
        {
            struct timeval tv;
            gettimeofday( &tv, NULL );
            return ( tv.tv_sec );
        }

        static void get_time_string( char * buffer, size_t buffer_size )
        {
            time_t    now;
            struct tm timeinfo;

            time( &now );
            localtime_r( &now, &timeinfo );
            strftime( buffer, buffer_size, "%Y-%m-%d %H:%M:%S %Z", &timeinfo );
        }

        void set_timezone( const char * timezone )
        {
            if ( timezone == nullptr || strlen( timezone ) == 0 ) return;
            xSemaphoreTake( _timeMtx, portMAX_DELAY );
            if ( setenv( "TZ", timezone, 1 ) != 0 )
            {
                ESP_LOGE( TAG, "Failed to set timezone. Invalid timezone format: %s", timezone );
            }
            else
            {
                _timezone = timezone;
                tzset();

                time( &_now );
                localtime_r( &_now, &_timeinfo );
                ESP_LOGI( TAG, "The current date/time is: %s", asctime( &_timeinfo ) );
            }
            xSemaphoreGive( _timeMtx );
        }

    private:
        static constexpr uint32_t UPDATED_INTERVAL = 5000;
        SemaphoreHandle_t         _timeMtx         = xSemaphoreCreateMutex();

        void initialize_sntp( void )
        {
            ESP_LOGI( TAG, "Initializing SNTP" );
            esp_sntp_setoperatingmode( SNTP_OPMODE_POLL );    // TODO: otestovat
            esp_sntp_setservername( 0, "pool.ntp.org" );
            esp_sntp_set_sync_mode( SNTP_SYNC_MODE_IMMED );
            esp_sntp_set_time_sync_notification_cb( time_sync_notification_cb );
            esp_sntp_init();
        }

        static void time_sync_notification_cb( struct timeval * tv )
        {
            time_t    now;
            char      strftime_buf[ 64 ];
            struct tm timeinfo;

            time( &now );
            localtime_r( &now, &timeinfo );
            strftime( strftime_buf, sizeof( strftime_buf ), "%c", &timeinfo );
            // ESP_LOGI( "RTC", "Timestamp: %ld", (u32) now );
            // ESP_LOGI( TAG, "Time synchronized: %s", strftime_buf );
        }

        // default timezone
        const char * _timezone = "CET-1CEST,M3.5.0,M10.5.0/3";
        time_t       _now;
        struct tm    _timeinfo;
    };
}    // namespace AsnPlus::Esp32

#endif

#ifndef _COMPONENTS_ESP32_HW_TIMER_HPP
#define _COMPONENTS_ESP32_HW_TIMER_HPP

#include "esp_timer.h"

namespace AsnPlus
{
    class HwTimer
    {
    public:
        esp_timer_handle_t _timer     = {};
        volatile bool      is_elapsed = false;

        void initialize( u32 microseconds )
        {
            esp_timer_create_args_t timer_args {};

            timer_args.callback = &callback, timer_args.arg = this,

            esp_timer_create( &timer_args, &_timer );
            esp_timer_start_periodic( _timer, microseconds );
        }

        static void callback( void * arg )
        {
            HwTimer * self   = static_cast< HwTimer * >( arg );
            self->is_elapsed = true;
        }
    };
}    // namespace AsnPlus

#endif

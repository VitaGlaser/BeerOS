#ifndef _ASNPLUS_ESP32_PWM_HPP
#define _ASNPLUS_ESP32_PWM_HPP

#include "asn/asn-core/types.hpp"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

namespace AsnPlus::Esp32
{
    class Pwm
    {
    public:
        Pwm( gpio_num_t       gpio,
             ledc_timer_bit_t resolution = LEDC_TIMER_8_BIT,
             ledc_timer_t     timer_num  = LEDC_TIMER_0,
             ledc_channel_t   channel    = LEDC_CHANNEL_0,
             u32              frequency  = 1000,
             u32              duty_cycle = 0 ) :
            _gpio { gpio },
            _resolution { resolution },
            _timer { timer_num },
            _channel { channel },
            _frequency { frequency },
            _duty_cycle { duty_cycle }
        {
        }

        void initialize()
        {
            _ledc_timer.duty_resolution = _resolution;
            ledc_timer_config( &_ledc_timer );
            ledc_channel_config( &_ledc_channel );
            ledc_fade_func_install( 0 );
        }

        void setFrequency( u32 frequency )
        {
            _frequency = frequency;
            _updateTimer();
        }

        void setDutyCycle( u32 duty_cycle )
        {
            _duty_cycle = duty_cycle;
            _update_duty_cycle();
        }

        void fadeToDuty( u32 target_duty, u32 fade_time_ms )
        {
            _duty_cycle = target_duty;
            ledc_set_fade_with_time( _ledc_channel.speed_mode, _ledc_channel.channel, target_duty, fade_time_ms );
            ledc_fade_start( _ledc_channel.speed_mode, _ledc_channel.channel, LEDC_FADE_NO_WAIT );
        }

        void fadeToDutyByStep( u32 target_duty, u32 step_num, u32 duty_inc_per_step )
        {
            _duty_cycle = target_duty;

            ledc_set_fade_with_step(
                _ledc_channel.speed_mode, _ledc_channel.channel, target_duty, duty_inc_per_step, step_num
            );

            ledc_fade_start( _ledc_channel.speed_mode, _ledc_channel.channel, LEDC_FADE_NO_WAIT );
        }

        u32 getDutyCycle() { return _duty_cycle; }

        ledc_timer_bit_t getDutyResolution()
        {
            return _resolution;
        }

        void stop() { ledc_stop( _ledc_channel.speed_mode, _ledc_channel.channel, 0 ); }

    private:
        void _updateTimer()
        {
            _ledc_timer.freq_hz = _frequency;
            ledc_timer_config( &_ledc_timer );
        }

        void _updateChannel()
        {
            _ledc_channel.duty = _duty_cycle;
            ledc_channel_config( &_ledc_channel );
        }

        void _update_duty_cycle()
        {
            ledc_set_duty( _ledc_channel.speed_mode, _ledc_channel.channel, _duty_cycle );
            ledc_update_duty( _ledc_channel.speed_mode, _ledc_channel.channel );
        }

        gpio_num_t       _gpio;
        ledc_timer_bit_t _resolution;
        ledc_timer_t     _timer;
        ledc_channel_t   _channel;
        u32              _frequency;
        u32              _duty_cycle;

        // Configuration for the timer
        ledc_timer_config_t _ledc_timer = {
            .speed_mode      = LEDC_LOW_SPEED_MODE,    // Timer mode
            .duty_resolution = _resolution,            // Resolution of PWM duty
            .timer_num       = _timer,                 // Timer index
            .freq_hz         = _frequency,             // Frequency of PWM signal
            .clk_cfg         = LEDC_AUTO_CLK,          // Auto select the source clock
            .deconfigure {},
        };

        // Configuration for the LEDC channel
        ledc_channel_config_t _ledc_channel = {
            .gpio_num   = _gpio,    // GPIO number
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel    = _channel,
            .intr_type  = LEDC_INTR_DISABLE,
            .timer_sel  = _timer,
            .duty       = _duty_cycle,    // Set duty to 50%. (2 ** 13) * 50% = 4096
            .hpoint {},
            .flags {},
        };
    };
}    // namespace AsnPlus::Esp32

#endif

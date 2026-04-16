#pragma once

#include "asn/asn-core/color.hpp"
#include "asn/asn-core/timer.hpp"
#include "asn/asn-core/types.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "led_strip_rmt.h"
#include "led_strip_types.h"
#include <stdio.h>
#include <vector>

namespace AsnPlus::Drivers
{
    class LedStrip
    {
    public:
        LedStrip( gpio_num_t controlPin, u8 countOfLeds, bool invertOutput ) :
            _controlPin( controlPin ),
            _countOfLeds( countOfLeds ),
            _ledColors( countOfLeds, Colors::NONE ),
            _ledColorsBrightness( countOfLeds, 255 ),
            _invertOutput( invertOutput )
        {
        }

        u8 blink_count     = 0;
        u8 fade_step_count = 0;

        void initialize()
        {
            _led_strip = configure();
            clear( 1, _countOfLeds );
        }

        void poll() { _updateStrip(); }

        void blink( u8 led_number, Color color, uint32_t timeout_ms )
        {
            if ( _blinkTimer.isElapsed() )
            {
                if ( _led_on_off )
                {
                    blink_count++;
                    setColor( led_number, color );
                }
                else
                {
                    clear( led_number );
                    if ( blink_count >= 255 ) blink_count = 0;
                }

                _led_on_off = ! _led_on_off;
                _blinkTimer.start( timeout_ms );
            }
        }

        void blink_fade( u8 led_number, Color color, uint32_t timeout_ms, u8 fade_steps )
        {
            if ( _blinkTimer.isElapsed() )
            {
                if ( _led_on_off )
                {
                    fade_step_count++;
                    setColor( led_number, _current_color, fade_step_count * 255 / fade_steps );

                    if ( fade_step_count >= fade_steps )
                    {
                        blink_count++;
                        fade_step_count = 0;
                        _blinkTimer.start( timeout_ms );
                        _led_on_off = ! _led_on_off;
                    }
                }
                else
                {
                    fade_step_count++;
                    setColor( led_number, _current_color, 255 - fade_step_count * 255 / fade_steps );

                    if ( fade_step_count >= fade_steps )
                    {
                        if ( color != _current_color ) _current_color = color;
                        fade_step_count = 0;
                        _blinkTimer.start( timeout_ms );
                        _led_on_off = ! _led_on_off;
                    }
                }
            }
        }

        void blink_all( Color color, uint32_t timeout_ms )
        {
            if ( _blinkTimer.isElapsed() )
            {
                if ( _led_on_off )
                {
                    blink_count++;
                    fill( 0, _countOfLeds, color );
                }
                else
                {
                    clear( 0, _countOfLeds );
                    if ( blink_count >= 255 ) blink_count = 0;
                }

                _led_on_off = ! _led_on_off;
                _blinkTimer.start( timeout_ms );
            }
        }

        void blink_all( Color color, uint32_t timeout_ms, float on_ratio = 0.5f )
        {
            if ( _blinkTimer.isElapsed() )
            {
                uint32_t on_ms  = static_cast< uint32_t >( timeout_ms * on_ratio );
                uint32_t off_ms = timeout_ms - on_ms;

                if ( _led_on_off )
                {
                    blink_count++;
                    fill( 0, _countOfLeds, color );
                    _blinkTimer.start( on_ms );
                }
                else
                {
                    clear( 0, _countOfLeds );
                    if ( blink_count >= 255 ) blink_count = 0;
                    _blinkTimer.start( off_ms );
                }
                _led_on_off = ! _led_on_off;
            }
        }

        void setColor( u8 led_number, Color color, u8 brightness = 255 )
        {
            if ( led_number < _countOfLeds )
            {
                _ledColors[ led_number ]           = color;
                _ledColorsBrightness[ led_number ] = brightness;
            }
        }

        void clear( u8 led_number ) { setColor( led_number, Colors::NONE ); }

        void clear( u8 start, u8 length, int exception = -1 )
        {
            for ( u8 i = start; i < start + length; i++ )
            {
                if ( i != exception )
                {
                    clear( i );
                }
            }
        }

        void fill( u8 start, u8 length, Color color, u8 brightness = 255 )
        {
            for ( u8 i = start; i < start + length; i++ )
            {
                setColor( i, color, brightness );
            }
        }

        void clear_all() { clear( 0, _countOfLeds ); }

        uint8_t get_count() { return _countOfLeds; }

    private:
        void _updateStrip()
        {
            for ( u8 i = 0; i < _countOfLeds; i++ )
            {
                Color color      = _ledColors[ i ];
                u8    brightness = _ledColorsBrightness[ i ];
                u8    red        = color.red;
                u8    green      = color.green;
                u8    blue       = color.blue;

                red              = ( red * brightness ) / UINT8_MAX;
                green            = ( green * brightness ) / UINT8_MAX;
                blue             = ( blue * brightness ) / UINT8_MAX;

                led_strip_set_pixel( _led_strip, i, red, green, blue );
            }

            led_strip_refresh( _led_strip );
        }

        led_strip_handle_t configure()
        {
            led_strip_config_t strip_config = {
                .strip_gpio_num = _controlPin, // The GPIO that connected to the LED strip's data line
                .max_leds       = _countOfLeds, // The number of LEDs in the strip,
                .led_model      = LED_MODEL_WS2812, // LED strip model
                .flags          = {
                                   .invert_out = _invertOutput,       // whether to invert the output signal
                },
            };

            // led_strip_spi_config_t spi_config = {
            //     .clk_src = SPI_CLK_SRC_DEFAULT, // different clock source can lead to different power consumption
            //     .spi_bus = SPI2_HOST, // SPI bus ID
            //     .flags   = {
            //                 .with_dma = true,            // Using DMA can improve performance and help drive more
            //                 LEDs
            //     },
            // };

            // led_strip_new_spi_device( &strip_config, &spi_config, &_led_strip );

            led_strip_rmt_config_t rmt_config = {
                .clk_src = RMT_CLK_SRC_DEFAULT, // different clock source can lead to different power consumption
                .resolution_hz     = 10 * 1000 * 1000, // RMT counter clock frequency
                .mem_block_symbols = 0, // the memory block size used by the RMT channel
                .flags             = {
                                      .with_dma = true,                     // Using DMA can improve performance when driving more LEDs
                }
            };
            led_strip_new_rmt_device( &strip_config, &rmt_config, &_led_strip );
            return _led_strip;
        }

        gpio_num_t           _controlPin;
        led_strip_handle_t   _led_strip;
        bool                 _led_on_off    = false;
        Color                _current_color = Colors::NONE;
        Timer<>              _blinkTimer {};
        u8                   _countOfLeds;
        std::vector< Color > _ledColors;
        std::vector< u8 >    _ledColorsBrightness;
        bool                 _invertOutput;
    };
}    // namespace AsnPlus::Drivers

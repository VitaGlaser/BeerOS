#ifndef _COMPONENTS_ESP32_GPIO_HPP
#define _COMPONENTS_ESP32_GPIO_HPP

#include "asn/asn-core/io.hpp"
#include "driver/gpio.h"
#include "driver/gpio_filter.h"

namespace AsnPlus::Esp32
{
    class DigitalInput : public Input< bool >
    {
    public:
        using IsrCallback = void( IRAM_ATTR * )( void * arg );

        DigitalInput(
            gpio_num_t      gpio,
            bool            pull_up_enable,
            bool            pull_down_enable,
            gpio_int_type_t interruptType      = GPIO_INTR_DISABLE,
            IsrCallback     isrHandler         = nullptr,
            void *          isrHandlerArg      = nullptr,
            bool            glitchFilterEnable = false
        ) :
            _gpio( gpio ),
            _isrHandler( isrHandler )
        {
            gpio_config_t io_conf;
            io_conf.intr_type    = interruptType;
            io_conf.mode         = GPIO_MODE_INPUT;
            io_conf.pin_bit_mask = ( 1ULL << _gpio );
            io_conf.pull_down_en = pull_down_enable ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
            io_conf.pull_up_en   = pull_up_enable ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
            gpio_config( &io_conf );

            if ( interruptType != GPIO_INTR_DISABLE && isrHandler != nullptr )
            {
                gpio_install_isr_service( 0 );
                gpio_isr_handler_add( _gpio, _isrHandler, isrHandlerArg );
            }

            if ( glitchFilterEnable )
            {
                gpio_pin_glitch_filter_config_t config = {
                    .clk_src  = GLITCH_FILTER_CLK_SRC_DEFAULT,
                    .gpio_num = _gpio,
                };
                ESP_ERROR_CHECK( gpio_new_pin_glitch_filter( &config, &_glitchFilter ) );
                ESP_ERROR_CHECK( gpio_glitch_filter_enable( _glitchFilter ) );
            }
        }

        virtual bool read() { return gpio_get_level( _gpio ); }

    private:
        gpio_num_t                  _gpio;
        IsrCallback                 _isrHandler;
        gpio_glitch_filter_handle_t _glitchFilter;
    };

    class DigitalOutput : public Output< bool >
    {
    public:
        DigitalOutput( gpio_num_t gpio, bool pull_up_enable, bool pull_down_enable ) : _gpio( gpio )
        {
            gpio_config_t io_conf;
            io_conf.intr_type    = GPIO_INTR_DISABLE;
            io_conf.mode         = GPIO_MODE_OUTPUT;
            io_conf.pin_bit_mask = ( 1ULL << _gpio );
            io_conf.pull_down_en = pull_down_enable ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
            io_conf.pull_up_en   = pull_up_enable ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
            gpio_config( &io_conf );
        }

        virtual void write( bool value ) { gpio_set_level( _gpio, value ); }

    private:
        gpio_num_t _gpio;
    };
}    // namespace AsnPlus::Esp32

#endif

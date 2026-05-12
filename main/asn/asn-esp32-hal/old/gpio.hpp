#ifndef _COMPONENTS_ESP32_GPIO_HPP
#define _COMPONENTS_ESP32_GPIO_HPP

#include "asn/asn-core/io.hpp"
#include "driver/gpio.h"

namespace AsnPlus::Esp32
{
    class DigitalInput : public Input< bool >
    {
    public:
        DigitalInput( gpio_num_t gpio, bool pull_up_enable, bool pull_down_enable ) : _gpio( gpio )
        {
            gpio_config_t io_conf;
            io_conf.intr_type    = GPIO_INTR_DISABLE;
            io_conf.mode         = GPIO_MODE_INPUT;
            io_conf.pin_bit_mask = ( 1ULL << _gpio );
            io_conf.pull_down_en = pull_down_enable ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
            io_conf.pull_up_en   = pull_up_enable ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
            gpio_config( &io_conf );
        }

        virtual bool read() { return gpio_get_level( _gpio ); }

    private:
        gpio_num_t _gpio;
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

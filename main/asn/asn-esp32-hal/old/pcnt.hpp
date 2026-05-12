#ifndef _COMPONENTS_ESP32_PCNT_HPP
#define _COMPONENTS_ESP32_PCNT_HPP

#include "asn/asn-core/types.hpp"
#include "asn/asn-core/vector.hpp"
#include "driver/gpio.h"
#include "driver/pulse_cnt.h"
#include "esp_log.h"

namespace AsnPlus::Esp32
{
    class Pcnt
    {
        static constexpr const char * TAG        = "PCNT";

        using Callback                           = pcnt_watch_cb_t;
        static constexpr size_t MAX_WATCH_POINTS = 4;

        gpio_num_t _pulse_gpio;
        gpio_num_t _ctrl_gpio;
        int32_t    _min_count;
        int32_t    _max_count;

        pcnt_unit_handle_t    _pcnt_unit;
        pcnt_channel_handle_t _pcnt_channel;

    public:
        Pcnt(
            gpio_num_t pulse_gpio,
            gpio_num_t ctrl_gpio = GPIO_NUM_NC,
            int32_t    min_count = -1,
            int32_t    max_count = 32'767
        ) :
            _pulse_gpio( pulse_gpio ),
            _ctrl_gpio( ctrl_gpio ),
            _min_count( min_count ),
            _max_count( max_count ),
            _pcnt_unit( nullptr ),
            _pcnt_channel( nullptr )
        {
        }

        void initialize()
        {
            pcnt_unit_config_t unit_config = {
                .low_limit = _min_count, .high_limit = _max_count, .intr_priority = 0, .flags = { .accum_count = true }
            };

            ESP_ERROR_CHECK( pcnt_new_unit( &unit_config, &_pcnt_unit ) );

            pcnt_chan_config_t chan_config = {
                .edge_gpio_num = _pulse_gpio, .level_gpio_num = _ctrl_gpio, .flags = {}
            };

            ESP_ERROR_CHECK( pcnt_new_channel( _pcnt_unit, &chan_config, &_pcnt_channel ) );

            ESP_ERROR_CHECK( pcnt_channel_set_edge_action(
                _pcnt_channel, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD
            ) );

            ESP_ERROR_CHECK( pcnt_channel_set_level_action(
                _pcnt_channel, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_KEEP
            ) );

            ESP_ERROR_CHECK( pcnt_unit_enable( _pcnt_unit ) );
            ESP_ERROR_CHECK( pcnt_unit_start( _pcnt_unit ) );
            clear();
        }

        void set_callback( Callback callback, void * user_ctx )
        {
            pcnt_event_callbacks_t callbacks = { .on_reach = callback };

            ESP_ERROR_CHECK( pcnt_unit_register_event_callbacks( _pcnt_unit, &callbacks, user_ctx ) );
        }

        void add_watch_point( int value )
        {
            ESP_ERROR_CHECK( pcnt_unit_add_watch_point( _pcnt_unit, value ) );
            clear();
        }

        i32 read()
        {
            int value = 0;
            ESP_ERROR_CHECK( pcnt_unit_get_count( _pcnt_unit, &value ) );
            return static_cast< i32 >( value );
        }

        void clear() { ESP_ERROR_CHECK( pcnt_unit_clear_count( _pcnt_unit ) ); }

        void stop() { ESP_ERROR_CHECK( pcnt_unit_stop( _pcnt_unit ) ); }

        void resume() { ESP_ERROR_CHECK( pcnt_unit_start( _pcnt_unit ) ); }

        pcnt_unit_handle_t raw_handle() const { return _pcnt_unit; }
    };
}    // namespace AsnPlus::Esp32

#endif

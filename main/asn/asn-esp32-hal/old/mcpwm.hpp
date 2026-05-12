#ifndef PWM_HANDLER_HPP
#define PWM_HANDLER_HPP

#include "driver/gpio.h"
#include "driver/mcpwm_prelude.h"
#include "esp_err.h"
#include "freertos/queue.h"

namespace AsnPlus::Esp32
{
    // Struktura pro předávání dat z ISR do hlavní smyčky
    struct PwmAdcEvent
    {
        bool is_high_level;
    };

    class PwmHandler
    {
    public:
        PwmHandler( gpio_num_t pwm_gpio, QueueHandle_t queue ) : _pwm_gpio( pwm_gpio ), _event_queue( queue ) {}

        void initialize()
        {
            mcpwm_timer_handle_t timer_handle = NULL;
            mcpwm_timer_config_t timer_config = {
                .group_id      = 0,
                .clk_src       = MCPWM_TIMER_CLK_SRC_DEFAULT,
                .resolution_hz = 1'000'000, // Rozlišení časovače v Hz, např. 1 MHz
                .count_mode    = MCPWM_TIMER_COUNT_MODE_UP, // Režim počítání
                .period_ticks  = 1000, // Počet ticků na jednu periodu PWM (pro frekvenci 1 kHz)
                .intr_priority = 1,
                .flags         = {
                                  .update_period_on_empty = true,
                                  .update_period_on_sync  = false,
                                  },
            };
            mcpwm_new_timer( &timer_config, &timer_handle );

            mcpwm_oper_handle_t     operator_handle = NULL;
            mcpwm_operator_config_t operator_config = {
                .group_id = 0,
            };
            mcpwm_new_operator( &operator_config, &operator_handle );

            mcpwm_cmpr_handle_t       comparator_handle = NULL;
            mcpwm_comparator_config_t comparator_config = {
                .flags = {
                          .update_cmp_on_tep = true,
                          }
            };
            mcpwm_new_comparator( operator_handle, &comparator_config, &comparator_handle );

            // Připojte operátor k časovači
            mcpwm_operator_connect_timer( operator_handle, timer_handle );

            // Nastavení duty cycle (např. 50 %)
            mcpwm_comparator_set_compare_value( comparator_handle, 500 );

            // Konfigurace generátoru
            mcpwm_gen_handle_t       generator_handle = NULL;
            mcpwm_generator_config_t generator_config = {
                .gen_gpio_num = _pwm_gpio,
            };
            mcpwm_new_generator( operator_handle, &generator_config, &generator_handle );

            // Nastavení akcí generátoru pro časovač
            mcpwm_gen_timer_event_action_t timer_event_action_low = {
                .direction = MCPWM_TIMER_DIRECTION_UP,    // Směr časovače
                .event     = MCPWM_TIMER_EVENT_EMPTY,     // Event když časovač dosáhne 0
                .action    = MCPWM_GEN_ACTION_LOW         // Akce nastavení na LOW
            };
            mcpwm_generator_set_action_on_timer_event( generator_handle, timer_event_action_low );

            // Nastavení akcí generátoru pro komparátor
            mcpwm_gen_compare_event_action_t compare_event_action_high = {
                .direction  = MCPWM_TIMER_DIRECTION_UP,    // Směr časovače
                .comparator = comparator_handle,           // Použitý komparátor
                .action     = MCPWM_GEN_ACTION_HIGH        // Akce nastavení na HIGH
            };
            mcpwm_generator_set_action_on_compare_event( generator_handle, compare_event_action_high );

            // Registrace callbacků pro detekci plné a prázdné hodnoty časovače
            // mcpwm_timer_event_callbacks_t cbs =
            // {
            //     .on_full = on_pwm_high,  // Callback při dosažení hodnoty komparátoru (HIGH)
            //     .on_empty = on_pwm_low,  // Callback při dosažení 0 (LOW)
            // };
            // mcpwm_timer_register_event_callbacks(timer_handle, &cbs, this);

            // Spuštění časovače
            mcpwm_timer_enable( timer_handle );
            mcpwm_timer_start_stop( timer_handle, MCPWM_TIMER_START_NO_STOP );
        }

    private:
        // static bool IRAM_ATTR on_pwm_high(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t *edata, void
        // *user_data)
        // {
        //     PwmHandler* handler = static_cast<PwmHandler*>(user_data);
        //     handler->queue_adc_value(true);  // Měření ADC na pinu při vysoké úrovni
        //     return true;
        // }

        // static bool IRAM_ATTR on_pwm_low(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t *edata, void
        // *user_data)
        // {
        //     PwmHandler* handler = static_cast<PwmHandler*>(user_data);
        //     handler->queue_adc_value(false);  // Měření ADC na pinu při nízké úrovni
        //     return true;
        // }

        // void IRAM_ATTR queue_adc_value(bool is_high_level)
        // {
        //     PwmAdcEvent event = { .is_high_level = is_high_level };
        //     BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        //     xQueueSendFromISR(_event_queue, &event, &xHigherPriorityTaskWoken);
        // }

        gpio_num_t    _pwm_gpio;
        QueueHandle_t _event_queue;
    };
}    // namespace AsnPlus::Esp32

#endif    // PWM_HANDLER_HPP

#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/logger.hpp"

#include "asn/asn-hal/include/peripherals/gpio.hpp"

#include "driver/gpio.h"

namespace AsnPlus::Esp32
{
    class Gpio : public IGpio
    {
    public:
        struct Config : public IGpio::Config
        {
            gpio_num_t pin;
            bool       pullUpEnabled   = false;
            bool       pullDownEnabled = false;
        };

        Gpio( Config & config );

        bool initialize() override;
        void poll() override;
        void set( bool value ) override;
        bool get() override;
        void toggle() override;
        bool enableInterrupt() override;
        bool disableInterrupt() override;

    private:
        static constexpr const char TAG[] = "Gpio";
        using Log                         = Logger< ModuleConfig::Esp32::LOG_LEVEL, TAG >;

        Config & _config;
        bool     _state = false;

        static bool installService();
        static gpio_int_type_t mapInterruptType( Config::InterruptType t );
        static void isrHandler( void * arg );
    };

}    // namespace AsnPlus::Esp32

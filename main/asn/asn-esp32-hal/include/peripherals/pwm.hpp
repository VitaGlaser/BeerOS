#pragma once

#include "asn_module_config.hpp"

#include "driver/gpio.h"
#include "driver/mcpwm_prelude.h"

#include "esp_err.h"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/vector.hpp"

#include "asn/asn-hal/include/peripherals/pwm.hpp"

namespace AsnPlus::Esp32
{
    class Pwm : public IPwm
    {
    public:
        struct Config : public IPwm::Config
        {
            uint32_t resolutionHz = 1'000'000;
        };

        // MARK: Channel
        class Channel : public IPwm::IChannel
        {
        public:
            // No extra fields:
            // - channel == gpio_num (cast through uint8_t)
            // - inverted used as-is
            struct Config : public IPwm::IChannel::Config
            {
            };

            Channel( Config & config, Pwm & unit );

            bool initialize() override;
            void setDutyCycle( float dutyCycle ) override;
            float getDutyCycle() override;
            IPwm::IChannel::Config getConfig() override;

        private:
            static constexpr const char TAG[] = "Pwm::Channel";
            using Log                         = Logger< ModuleConfig::Esp32::LOG_LEVEL, TAG >;

            Config & _config;
            Pwm &    _unit;

            mcpwm_cmpr_handle_t _comparator = nullptr;
            mcpwm_gen_handle_t  _generator  = nullptr;

            float _duty                     = 0.0f;

            void _applyDuty( float duty );
        };

        Pwm( Config & config );

        bool initialize() override;
        void poll() override;
        bool start() override;
        bool stop() override;
        void setFrequencyHz( uint32_t frequencyHz ) override;
        uint32_t getFrequencyHz() override;

    private:
        static constexpr const char TAG[] = "Pwm";
        using Log                         = Logger< ModuleConfig::Esp32::LOG_LEVEL, TAG >;

        Config & _config;

        mcpwm_timer_handle_t _timer = nullptr;
        mcpwm_oper_handle_t  _oper  = nullptr;

        uint32_t _periodTicks       = 0;
    };

}    // namespace AsnPlus::Esp32

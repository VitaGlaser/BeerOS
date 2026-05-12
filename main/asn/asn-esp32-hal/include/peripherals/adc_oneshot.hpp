#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-hal/include/peripherals/adc_oneshot.hpp"

#include "asn/asn-core/logger.hpp"

#include "esp_adc/adc_oneshot.h"

namespace AsnPlus::Esp32
{
    class AdcOneshot : public IAdcOneshot
    {
    public:
        class Channel : public IChannel
        {
        public:
            struct Config : public IChannel::Config
            {
            };

            Channel( IChannel::Config & config );

            IChannel::Config getConfig() override;
        };

        AdcOneshot( IAdcOneshot::Config config );

        bool     initialize() override;
        bool     addChannel( IChannel & channel ) override;
        uint32_t readRaw( IChannel & channel ) override;
        uint32_t readVoltage( IChannel & channel ) override;

    protected:
    private:
        static constexpr const char TAG[] = "AdcOneshot";
        using Log                         = Logger< ModuleConfig::Esp32::LOG_LEVEL, TAG >;

        adc_oneshot_unit_handle_t _adcHandle {};
        adc_cali_handle_t         _caliHandle {};

        bool calibrate();
    };
}    // namespace AsnPlus::Esp32

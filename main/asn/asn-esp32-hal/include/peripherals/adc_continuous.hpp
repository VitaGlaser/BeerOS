#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/logger.hpp"

#include "asn/asn-hal/include/peripherals/adc_continuous.hpp"

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_continuous.h"

namespace AsnPlus::Esp32
{
    class AdcContinuous : public IAdcContinuous
    {
    public:
        class Channel : public IChannel
        {
        public:
            struct Config : public IChannel::Config
            {
            };

            Channel( Config & config, IRingBuffer< uint32_t > & buffer, IAdcContinuous & adc );

            bool initialize() override;
            IChannel::Config getConfig() override;
        };

        struct Config : public IAdcContinuous::Config
        {
            uint16_t dmaBufferSize;
            uint16_t conversionFrameSize;
        };

        AdcContinuous( Config & config );

        bool initialize() override;
        void poll() override;
        bool start() override;
        bool stop() override;

    protected:
    private:
        static constexpr const char TAG[]      = "AdcContinuous";
        using Log                              = Logger< ModuleConfig::Esp32::LOG_LEVEL, TAG >;

        static constexpr uint16_t ADC_READ_LEN = 1024;

        Config & _config;

        adc_continuous_handle_t   _adcHandle = nullptr;
        adc_digi_pattern_config_t _adcPattern[ MAX_CHANNELS ];
        adc_cali_handle_t         _calibrationHandle;

        uint8_t  _buffer[ ADC_READ_LEN ];
        uint32_t _bufferLength = 0;
    };
}    // namespace AsnPlus::Esp32

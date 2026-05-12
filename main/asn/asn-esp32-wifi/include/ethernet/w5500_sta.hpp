#pragma once

// W5500 SPI Ethernet driver layer — hardware init, MAC/PHY, link state.
#include "asn_module_config.hpp"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_eth_com.h"
#include "esp_eth_driver.h"
#include "esp_eth_mac_spi.h"
#include "esp_eth_spec.h"
#include "esp_event.h"
#include "esp_mac.h"
#include "hal/spi_types.h"
#include "soc/gpio_num.h"

#include "asn/asn-core/logger.hpp"

#include "asn/asn-hal/include/common/common_structs.hpp"

namespace AsnPlus::Network
{
    class W5500Sta
    {
    public:
        struct Config : AsnPlus::Config
        {
            spi_host_device_t spiHost       = SPI2_HOST;
            gpio_num_t        mosi          = GPIO_NUM_NC;
            gpio_num_t        miso          = GPIO_NUM_NC;
            gpio_num_t        sck           = GPIO_NUM_NC;
            gpio_num_t        cs            = GPIO_NUM_NC;
            gpio_num_t        nRst          = GPIO_NUM_NC;
            gpio_num_t        intPin        = GPIO_NUM_NC;
            int               clockSpeedMHz = 20;
        };

        struct Runtime : AsnPlus::Runtime
        {
            bool    linkUp = false;
            uint8_t mac[ ETH_ADDR_LEN ] {};
        };

        explicit W5500Sta( Config & config, Runtime & runtime ) : _config( config ), _runtime( runtime ) {}

        void initialize();

        void             start();
        esp_eth_handle_t getEthHandle();
        const Runtime &  getRuntime() const;

    private:
        static constexpr const char TAG[] = "W5500Sta";
        using Log                         = AsnPlus::Logger< ModuleConfig::Network::LOG_LEVEL, TAG >;

        Config &         _config;
        Runtime &        _runtime;
        esp_eth_handle_t _ethHandle = nullptr;

        static void _ethEventCb( void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data )
        {
            static_cast< W5500Sta * >( arg )->_ethEventHandler( event_id, event_data );
        }

        void _ethEventHandler( int32_t event_id, void * event_data );
        bool _initSpi();
        bool _configureMacAddress();
    };

}    // namespace AsnPlus::Network

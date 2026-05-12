#pragma once

#include "asn_module_config.hpp"

#include "esp_netif.h"

namespace AsnPlus::Esp32
{
    // ! This needs to be intialized before any network interfaces are created
    class Netif
    {
    public:
        static void initialize()
        {
            ESP_ERROR_CHECK( esp_netif_init() );
            ESP_ERROR_CHECK( esp_event_loop_create_default() );
        }
    };
}
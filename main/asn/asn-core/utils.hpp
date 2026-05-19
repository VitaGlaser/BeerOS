#pragma once

#include <cstdint>

// TODO (Kostik): Split into vendor specific files for each company, espressif, stm, etc.

#if defined( STM32C0xx ) || defined( STM32C071xx ) || defined( STM32F0xx ) || defined( STM32F1xx ) ||                  \
    defined( STM32F2xx ) || defined( STM32F3xx ) || defined( STM32F4xx ) || defined( STM32F7xx ) ||                    \
    defined( STM32G0xx ) || defined( STM32G4xx ) || defined( STM32H5xx ) || defined( STM32H7xx ) ||                    \
    defined( STM32L0xx ) || defined( STM32L1xx ) || defined( STM32L4xx ) || defined( STM32L5xx ) ||                    \
    defined( STM32U0xx ) || defined( STM32U5xx ) || defined( STM32WBxx ) || defined( STM32WLxx )
#define STM32 1
#endif

// Platform detection
#if defined( ESP_PLATFORM )

extern "C" int64_t esp_timer_get_time();

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#define ASN_ESP32

#elif defined( STM32 )

#include "stm32c0xx_hal.h"
#else
#error "Unsupported platform. Please define ESP_PLATFORM or STM32."
#endif

namespace AsnPlus
{
    class Utils
    {
    public:
        static uint32_t getMs();
        static uint64_t getMs64();
        static void     delay( uint32_t ms );
    };
}    // namespace AsnPlus

#ifdef ASN_ESP32

inline uint32_t AsnPlus::Utils::getMs()
{
    return esp_timer_get_time() / 1000;    // Convert microseconds to milliseconds
}

inline uint64_t AsnPlus::Utils::getMs64()
{
    return esp_timer_get_time() / 1000;    // Convert microseconds to milliseconds
}

inline void AsnPlus::Utils::delay( uint32_t ms )
{
    vTaskDelay( pdMS_TO_TICKS( ms ) );
}

#endif    // ESP32

#ifdef STM32

inline uint32_t AsnPlus::Utils::getMs()
{
    return HAL_GetTick();
}

inline void AsnPlus::Utils::delay( uint32_t ms )
{
    HAL_Delay( ms );
}
#endif    // STM32

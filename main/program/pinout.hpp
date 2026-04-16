#pragma once

#include "asn/asn-esp32-hal/peripherals/gpio.hpp"

#include "asn/asn-expander-lib/expander.hpp"

namespace AsnPlus
{
    struct Pinout
    {
        static constexpr gpio_num_t

            ETH_nRST = gpio_num_t::GPIO_NUM_40,
            ETH_nINT = gpio_num_t::GPIO_NUM_39, ETH_CS = gpio_num_t::GPIO_NUM_41, ETH_MOSI = gpio_num_t::GPIO_NUM_45,
            ETH_MISO = gpio_num_t::GPIO_NUM_38, ETH_SCK = gpio_num_t::GPIO_NUM_21,

            RTU_TERM_EN = gpio_num_t::GPIO_NUM_10, RTU_TERM_TX = gpio_num_t::GPIO_NUM_48,
            RTU_TERM_RX = gpio_num_t::GPIO_NUM_15, RTU_TERM_DIR = gpio_num_t::GPIO_NUM_42,

            LED_DATA      = gpio_num_t::GPIO_NUM_14,

            EXPANDER_NRST = gpio_num_t::GPIO_NUM_9, EXPANDER_BOOT = gpio_num_t::GPIO_NUM_47,

            EXPANDER_CS = gpio_num_t::GPIO_NUM_46, EXPANDER_SPI_MOSI = gpio_num_t::GPIO_NUM_11,
            EXPANDER_SPI_MISO = gpio_num_t::GPIO_NUM_13, EXPANDER_SPI_SCK = gpio_num_t::GPIO_NUM_12,

            I2C_SDA = gpio_num_t::GPIO_NUM_2, I2C_SCL = gpio_num_t::GPIO_NUM_1,

            // TODO: Set actual LTE modem UART pins
            LTE_RESET = gpio_num_t::GPIO_NUM_16, LTE_UART_RX = gpio_num_t::GPIO_NUM_17,
            LTE_UART_TX                        = gpio_num_t::GPIO_NUM_18;

        static constexpr uint8_t LTE_POWER_KEY = 8;
    };
}    // namespace AsnPlus

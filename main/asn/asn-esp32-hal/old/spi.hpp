#ifndef _ASNPLUS_ESP32_SPI_HPP
#define _ASNPLUS_ESP32_SPI_HPP

#include "asn/asn-core/types.hpp"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstring>
#include <stdio.h>

namespace AsnPlus::Esp32
{
    class SPI
    {
    public:
        static constexpr u8 default_timeout_ms = 1000 / portTICK_PERIOD_MS;

        SPI( spi_host_device_t spi_host,
             gpio_num_t        mosi_pin,
             gpio_num_t        miso_pin,
             gpio_num_t        sclk_pin,
             gpio_num_t        cs_pin,
             u32               frequency,
             u8                spi_mode ) :
            _spi_host( spi_host ),
            _mosi_pin( mosi_pin ),
            _miso_pin( miso_pin ),
            _sclk_pin( sclk_pin ),
            _cs_pin( cs_pin ),
            _frequency( frequency ),
            _spi_mode( spi_mode )
        {
        }

        void initialize()
        {
            spi_bus_config_t bus_config = {
                .mosi_io_num     = _mosi_pin,
                .miso_io_num     = _miso_pin,
                .sclk_io_num     = _sclk_pin,
                .quadwp_io_num   = -1,
                .quadhd_io_num   = -1,
                .max_transfer_sz = 4096,
            };

            spi_device_interface_config_t dev_config = {
                .mode           = _spi_mode,
                .duty_cycle_pos = 128,
                .clock_speed_hz = (int) _frequency,
                .spics_io_num   = _cs_pin,
                .queue_size     = 1,
                .pre_cb         = nullptr,
                .post_cb        = nullptr,
            };

            spi_bus_initialize( _spi_host, &bus_config, SPI_DMA_CH_AUTO );
            spi_bus_add_device( _spi_host, &dev_config, &_spi_handle );
        }

        esp_err_t write( const u8 * write_buffer, size_t length_buffer, TickType_t ticks_to_wait = default_timeout_ms )
        {
            spi_transaction_t transaction = {
                .length    = length_buffer * 8,
                .tx_buffer = write_buffer,
                .rx_buffer = nullptr,
            };

            return spi_device_transmit( _spi_handle, &transaction );
        }

        esp_err_t read( u8 * read_buffer, size_t length_buffer, TickType_t ticks_to_wait = default_timeout_ms )
        {
            spi_transaction_t transaction = {
                .length    = length_buffer * 8,
                .tx_buffer = nullptr,
                .rx_buffer = read_buffer,
            };

            return spi_device_transmit( _spi_handle, &transaction );
        }

        esp_err_t write_read(
            const u8 * write_buffer,
            u8 *       read_buffer,
            size_t     length_buffer,
            TickType_t ticks_to_wait = default_timeout_ms
        )
        {
            spi_transaction_t transaction = {
                .length    = length_buffer * 8,
                .tx_buffer = write_buffer,
                .rx_buffer = read_buffer,
            };

            return spi_device_transmit( _spi_handle, &transaction );
        }

    private:
        spi_host_device_t   _spi_host;
        gpio_num_t          _mosi_pin;
        gpio_num_t          _miso_pin;
        gpio_num_t          _sclk_pin;
        gpio_num_t          _cs_pin;
        u32                 _frequency;
        spi_device_handle_t _spi_handle;
        u8                  _spi_mode;
    };
}    // namespace AsnPlus::Esp32

#endif

#pragma once

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#include "asn/asn-core/map.hpp"

#include "asn_module_config.hpp"

namespace AsnPlus::Esp32
{
    class I2C
    {
        static constexpr const char TAG[]     = "I2C Master";
        using Log                             = Logger< ModuleConfig::Esp32::LOG_LEVEL, TAG >;
        static constexpr u32 _default_timeout = -1;    // Autoselect timeout

        i2c_port_t _i2c_num;
        gpio_num_t _sda_pin;
        gpio_num_t _scl_pin;
        bool       _enable_pull_up;

        i2c_master_bus_handle_t _bus_handle;

        Map< uint16_t, i2c_master_dev_handle_t, ModuleConfig::Esp32::I2C_MAX_SLAVES > _devices;

    public:
        I2C( i2c_port_t i2c_num, gpio_num_t sda_pin, gpio_num_t scl_pin, bool enable_pull_up ) :
            _i2c_num( i2c_num ),
            _sda_pin( sda_pin ),
            _scl_pin( scl_pin ),
            _enable_pull_up( enable_pull_up )
        {
        }

        void initialize()
        {
            i2c_master_bus_config_t i2c_mst_config = {
                .i2c_port          = _i2c_num,
                .sda_io_num        = _sda_pin,
                .scl_io_num        = _scl_pin,
                .clk_source        = I2C_CLK_SRC_DEFAULT,
                .glitch_ignore_cnt = 7,
                .intr_priority     = 0, // Automatic priority selection
                .trans_queue_depth =
                    0, // Caution: Increasing value will enable asynchronous mode which is experimental
                .flags = {
                          .enable_internal_pullup = _enable_pull_up,
                          }
            };
            ESP_ERROR_CHECK( i2c_new_master_bus( &i2c_mst_config, &_bus_handle ) );
        }

        void add_device( u8 device_address, i2c_master_dev_handle_t * dev_handle, u32 frequency )
        {
            i2c_device_config_t dev_cfg = {
                .dev_addr_length = I2C_ADDR_BIT_LEN_7,
                .device_address  = device_address,
                .scl_speed_hz    = frequency,
                .scl_wait_us     = 0,
                .flags           = {
                                    .disable_ack_check = 0,
                                    }
            };
            ESP_ERROR_CHECK( i2c_master_bus_add_device( _bus_handle, &dev_cfg, dev_handle ) );
        }

        esp_err_t add_device( uint16_t device_address, uint32_t scl_speed_hz = 400'000 )
        {
            // Check if the device is already added
            if ( _devices.find( device_address ) != _devices.end() )
            {
                Log::warn("I2C device at address 0x%02X is already added", device_address );
                return ESP_OK;
            }

            i2c_device_config_t dev_config = {
                .dev_addr_length = I2C_ADDR_BIT_LEN_7,
                .device_address  = device_address,
                .scl_speed_hz    = scl_speed_hz,
                .scl_wait_us     = 0,
                .flags           = {
                                    .disable_ack_check = false,
                                    },
            };

            i2c_master_dev_handle_t device_handle;
            esp_err_t               ret = i2c_master_bus_add_device( _bus_handle, &dev_config, &device_handle );
            if ( ret != ESP_OK )
            {
                Log::error("Failed to add I2C device: %s", esp_err_to_name( ret ) );
                return ret;
            }

            _devices[ device_address ] = device_handle;
            Log::info("I2C device added successfully at address 0x%02X", device_address );
            return ESP_OK;
        }

        esp_err_t write(
            i2c_master_dev_handle_t dev_handle,
            const u8 *              write_buffer,
            size_t                  length_buffer,
            TickType_t              ticks_to_wait = _default_timeout
        )
        {
            return i2c_master_transmit( dev_handle, write_buffer, length_buffer, ticks_to_wait );
        }

        esp_err_t write( uint16_t device_address, const uint8_t * data, size_t length )
        {
            auto it = _devices.find( device_address );
            if ( it == _devices.end() )
            {
                Log::error("I2C device at address 0x%02X not found", device_address );
                return ESP_ERR_INVALID_ARG;
            }

            esp_err_t ret = i2c_master_transmit(
                it->second, data, length, pdMS_TO_TICKS( ModuleConfig::Esp32::I2C_MAX_READ_WAIT )
            );
            if ( ret != ESP_OK )
            {
                Log::error("Failed to write: %s", esp_err_to_name( ret ) );
                return ret;
            }
            return ret;
        }

        esp_err_t write_register( uint16_t device_address, uint8_t reg_addr, const uint8_t * data, size_t length )
        {
            // Check if the device exists
            auto it = _devices.find( device_address );
            if ( it == _devices.end() )
            {
                Log::error("I2C device at address 0x%02X not found", device_address );
                return ESP_ERR_INVALID_ARG;
            }

            uint8_t * buffer = (uint8_t *) malloc( length + 1 );
            if ( ! buffer )
            {
                Log::error("Memory allocation failed" );
                return ESP_ERR_NO_MEM;
            }

            buffer[ 0 ] = reg_addr;
            memcpy( &buffer[ 1 ], data, length );

            esp_err_t ret = i2c_master_transmit(
                it->second, buffer, length + 1, pdMS_TO_TICKS( ModuleConfig::Esp32::I2C_MAX_READ_WAIT )
            );
            free( buffer );

            if ( ret != ESP_OK )
            {
                Log::error("Failed to write to register 0x%02X: %s", reg_addr, esp_err_to_name( ret ) );
            }
            return ret;
        }

        esp_err_t read(
            i2c_master_dev_handle_t dev_handle,
            u8 *                    read_buffer,
            size_t                  length_buffer,
            TickType_t              ticks_to_wait = _default_timeout
        )
        {
            return i2c_master_receive( dev_handle, read_buffer, length_buffer, ticks_to_wait );
        }

        esp_err_t read( uint16_t device_address, uint8_t * data, size_t length )
        {
            auto it = _devices.find( device_address );
            if ( it == _devices.end() )
            {
                Log::error("I2C device at address 0x%02X not found", device_address );
                return ESP_ERR_INVALID_ARG;
            }

            esp_err_t ret =
                i2c_master_receive( it->second, data, length, pdMS_TO_TICKS( ModuleConfig::Esp32::I2C_MAX_READ_WAIT ) );
            if ( ret != ESP_OK )
            {
                Log::error("Failed to read: %s", esp_err_to_name( ret ) );
                return ret;
            }
            return ret;
        }

        esp_err_t read_register( uint16_t device_address, uint8_t reg_addr, uint8_t * data, size_t length )
        {
            // Check if the device exists
            auto it = _devices.find( device_address );
            if ( it == _devices.end() )
            {
                Log::error("I2C device at address 0x%02X not found", device_address );
                return ESP_ERR_INVALID_ARG;
            }

            esp_err_t ret = i2c_master_transmit_receive(
                it->second, &reg_addr, 1, data, length, pdMS_TO_TICKS( ModuleConfig::Esp32::I2C_MAX_READ_WAIT )
            );
            if ( ret != ESP_OK )
            {
                Log::error("Failed to read from register at address 0x%02X: %s", reg_addr, esp_err_to_name( ret ) );
                return ret;
            }
            return ret;
        }

        esp_err_t write_read(
            i2c_master_dev_handle_t dev_handle,
            const u8 *              write_buffer,
            size_t                  length_write_buffer,
            u8 *                    read_buffer,
            size_t                  length_read_buffer,
            TickType_t              ticks_to_wait = _default_timeout
        )
        {
            return i2c_master_transmit_receive(
                dev_handle, write_buffer, length_write_buffer, read_buffer, length_read_buffer, ticks_to_wait
            );
        }
    };
}    // namespace AsnPlus::Esp32

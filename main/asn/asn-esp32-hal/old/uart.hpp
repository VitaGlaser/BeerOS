#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/string.hpp"
#include "asn/asn-core/logger.hpp"

#include "driver/gpio.h"
#include "driver/uart.h"

namespace AsnPlus::Esp32
{
    class Uart
    {
    private:
        static constexpr const char TAG[] = "Uart";
        using Log                         = Logger< ModuleConfig::Esp32::LOG_LEVEL, TAG >;

        uart_port_t _port;
        gpio_num_t  _rx_pin;
        gpio_num_t  _tx_pin;
        int         _baud_rate;
        int         _rx_buffer_size;
        int         _tx_buffer_size;

    public:
        Uart(
            uart_port_t port,
            gpio_num_t  rx_pin,
            gpio_num_t  tx_pin,
            int         baud_rate      = 115'200,
            int         rx_buffer_size = 256,
            int         tx_buffer_size = 256
        ) :
            _port( port ),
            _rx_pin( rx_pin ),
            _tx_pin( tx_pin ),
            _baud_rate( baud_rate ),
            _rx_buffer_size( rx_buffer_size ),
            _tx_buffer_size( tx_buffer_size )
        {
        }

        void initialize()
        {
            Log::info( TAG, "Initializing UART" );
            uart_config_t uart_config = {
                .baud_rate           = _baud_rate,
                .data_bits           = UART_DATA_8_BITS,
                .parity              = UART_PARITY_DISABLE,
                .stop_bits           = UART_STOP_BITS_1,
                .flow_ctrl           = UART_HW_FLOWCTRL_DISABLE,
                .rx_flow_ctrl_thresh = 0,
                .source_clk          = UART_SCLK_DEFAULT,
                .flags               = { .allow_pd = 0, .backup_before_sleep = 0 }
            };

            ESP_ERROR_CHECK( uart_driver_install( _port, _rx_buffer_size, _tx_buffer_size, 0, nullptr, 0 ) );
            ESP_ERROR_CHECK( uart_param_config( _port, &uart_config ) );
            ESP_ERROR_CHECK( uart_set_pin( _port, _tx_pin, _rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE ) );
            Log::info( TAG, "UART initialized" );
        }

        esp_err_t set_baudrate( int baud_rate )
        {
            esp_err_t err = uart_set_baudrate( _port, baud_rate );
            if ( err == ESP_OK )
            {
                _baud_rate = baud_rate;
                Log::info( TAG, "Baud rate set to %d", baud_rate );
            }
            else
            {
                Log::error( TAG, "Failed to set baud rate: %s", esp_err_to_name( err ) );
            }
            return err;
        }

        int write( const char * data )
        {
            int ret = uart_write_bytes( _port, data, strlen( data ) );
            if ( ret < 0 ) Log::error( TAG, "Failed to write to UART: %s", esp_err_to_name( ret ) );
            return ret;
        }

        int write( const uint8_t * data, size_t len )
        {
            int ret = uart_write_bytes( _port, data, len );
            if ( ret < 0 ) Log::error( TAG, "Failed to write to UART: %s", esp_err_to_name( ret ) );
            return ret;
        }

        int read( uint8_t * buffer, size_t max_len, TickType_t timeout_ticks = pdMS_TO_TICKS( 50 ) )
        {
            int ret = uart_read_bytes( _port, buffer, max_len, timeout_ticks );
            if ( ret < 0 ) Log::error( TAG, "Failed to read from UART: %s", esp_err_to_name( ret ) );
            return ret;
        }

        void flush() { uart_flush( _port ); }
    };
}    // namespace AsnPlus::Esp32

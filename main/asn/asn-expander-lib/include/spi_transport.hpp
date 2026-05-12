#pragma once

#include "asn/asn-core/logger.hpp"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "hal/gpio_types.h"
#include "hal/spi_types.h"
#include "soc/gpio_num.h"
#include "transport.hpp"

namespace AsnPlus::Expander
{
    class SpiTransport : public Transport
    {
    public:
        SpiTransport( spi_host_device_t host, gpio_num_t cs, int frequency );

        void     initialize();
        uint16_t readRegister( uint16_t address ) override;
        void     writeRegister( uint16_t address, uint16_t data ) override;

        esp_err_t writeRead(
            const uint8_t * write_buffer,
            uint8_t *       read_buffer,
            size_t          length_buffer,
            TickType_t      ticks_to_wait = DEFAULT_TIMEOUT_MS / portTICK_PERIOD_MS
        );

    private:
        static constexpr const char TAG[] = "SpiTransport";
        using Log                         = Logger< 0, TAG >;

        spi_host_device_t   _host;
        gpio_num_t          _cs;
        int                 _frequency;
        spi_device_handle_t _device;

        static void _preCb( spi_transaction_t * trans );
        static void _postCb( spi_transaction_t * trans );
    };
}    // namespace AsnPlus::Expander

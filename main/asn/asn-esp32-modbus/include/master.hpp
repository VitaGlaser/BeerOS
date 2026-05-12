#pragma once

#include "asn_module_config.hpp"

#include "driver/uart.h"
#include "hal/uart_types.h"
#include "soc/gpio_num.h"

#include "esp_modbus_common.h"

// ! DONT REMOVE THIS
#ifdef atomic_int
#undef atomic_int
#endif

#include "esp_modbus_master.h"

#include "asn/asn-core/logger.hpp"

#include "structs.hpp"

namespace AsnPlus::Modbus
{
    class Master
    {
    public:
        Master() {}

    protected:
        static constexpr const char TAG[] = "Modbus::Master";
        using Log                         = Logger< ModuleConfig::Modbus::LOG_LEVEL, TAG >;

        void * _masterHandler = nullptr;
    };

    class RtuMaster : public Master
    {
    public:
        struct Config
        {
            uint32_t           baudrate          = 9600;
            uart_parity_t      parity            = UART_PARITY_DISABLE;
            uart_word_length_t dataBits          = UART_DATA_8_BITS;
            uart_stop_bits_t   stopBits          = UART_STOP_BITS_1;
            uint32_t           responseTimeoutMs = 1000;
        };

        RtuMaster( Config & config, uart_port_t port, gpio_num_t tx, gpio_num_t rx, gpio_num_t rts = GPIO_NUM_NC ) :
            _config( config ),
            _port( port ),
            _tx( tx ),
            _rx( rx ),
            _rts( rts )
        {
        }

        bool initialize();
        void test();
        bool request( mb_param_request_t * request, void * value );

    private:
        static constexpr const char TAG[] = "Modbus::RtuMaster";
        using Log                         = Logger< ModuleConfig::Modbus::LOG_LEVEL, TAG >;

        Config & _config;

        uart_port_t _port;
        gpio_num_t  _tx;
        gpio_num_t  _rx;
        gpio_num_t  _rts;

        // TODO: Fix device parameters properly, this is just a temporary fix
        const mb_parameter_descriptor_t device_parameters[ 1 ] = {
            // { CID, Param Name, Units, Modbus Slave Addr, Modbus Reg Type, Reg Start, Reg Size, Instance Offset, Data
            // Type, Data Size, Parameter Options, Access Mode}
            { 0,
             "Data_channel_0", "Volts",
             1, MB_PARAM_INPUT,
             0, 1,
             0, PARAM_TYPE_U16,
             2, 0,
             PAR_PERMS_READ_WRITE_TRIGGER }
        };
    };
}    // namespace AsnPlus::Modbus

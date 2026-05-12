#pragma once

#include "asn_module_config.hpp"

#include <cstdint>
#include <cstring>

#include "driver/uart.h"
#include "hal/uart_types.h"
#include "soc/gpio_num.h"

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_modbus_common.h"
#include "esp_modbus_slave.h"
#include "mb_port_types.h"
#include "mb_types.h"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/mutex.hpp"
#include "asn/asn-core/span.hpp"

#include "structs.hpp"

namespace AsnPlus::Modbus
{
    class Slave
    {
    public:
        Slave() {}

        void      addRegister( Register & reg );
        void      addArea( RegisterType type, uint16_t start_offset, Span< uint16_t > regs );
        esp_err_t postInit();
        void      setSlaveId(
                 const char * vendorName,
                 const char * productCode,
                 const char * majorMinorRevision,
                 const char * vendorUrl,
                 const char * productName,
                 const char * modelName,
                 const char * serialNumber
             );
        void     start();
        void     stop();
        uint64_t getLastMessageTimestampUs() const;

    protected:
        static constexpr uint32_t TASK_STACK_SIZE = 4096;

        void *                 _slaveHandler    = nullptr;
        TaskHandle_t           _eventTaskHandle = nullptr;
        mutable AsnPlus::Mutex _timestampMutex;
        uint64_t               _lastMessageUs = 0;

        static void _eventTask( void * arg );
        void        _startEventTask();
        void        _stopEventTask();

    private:
        static constexpr const char TAG[] = "Modbus::Slave";
        using Log                         = Logger< ModuleConfig::Modbus::LOG_LEVEL, TAG >;

        mb_param_type_t    _registerTypeToNative( RegisterType type );
        static mb_exception_t myCustomFcHandler( void * pinst, uint8_t * frame_ptr, uint16_t * plen );
    };

    class TcpSlave : public Slave
    {
    public:
        enum class IpType : uint8_t
        {
            IPV4 = 1,
            IPV6 = 2
        };

        TcpSlave( IpType ipType, uint16_t port, uint8_t uid = 0x01 ) : _ipType( ipType ), _port( port ), _uid( uid ) {}

        void initialize( void * netif );

    private:
        static constexpr const char TAG[] = "Modbus::TcpSlave";
        using Log                         = Logger< ModuleConfig::Modbus::LOG_LEVEL, TAG >;

        IpType   _ipType;
        uint16_t _port;
        uint8_t  _uid;
    };

    class RtuSlave : public Slave
    {
    public:
        RtuSlave(
            uart_port_t        port,
            gpio_num_t         tx,
            gpio_num_t         rx,
            gpio_num_t         rts,
            uint32_t           baudrate          = 9600,
            uart_parity_t      parity            = UART_PARITY_DISABLE,
            uint8_t            uid               = 0x01,
            uint16_t           responseTimeoutMs = 1000,
            uart_word_length_t dataBits          = UART_DATA_8_BITS,
            uart_stop_bits_t   stopBits          = UART_STOP_BITS_1
        ) :
            _port( port ),
            _tx( tx ),
            _rx( rx ),
            _rts( rts ),
            _baudrate( baudrate ),
            _parity( parity ),
            _uid( uid ),
            _responseTimeoutMs( responseTimeoutMs ),
            _dataBits( dataBits ),
            _stopBits( stopBits )
        {
        }

        void initialize();

    private:
        static constexpr const char TAG[] = "Modbus::RtuSlave";
        using Log                         = Logger< ModuleConfig::Modbus::LOG_LEVEL, TAG >;

        uart_port_t        _port;
        gpio_num_t         _tx;
        gpio_num_t         _rx;
        gpio_num_t         _rts;
        uint32_t           _baudrate;
        uart_parity_t      _parity;
        uint8_t            _uid;
        uint16_t           _responseTimeoutMs;
        uart_word_length_t _dataBits;
        uart_stop_bits_t   _stopBits;
    };

}    // namespace AsnPlus::Modbus

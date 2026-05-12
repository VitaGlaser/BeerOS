#pragma once

// #include "bootloader.hpp"
// #include "../spi_transport.hpp"
#include <cstddef>
#include <cstdint>
#include <sys/_stdint.h>

namespace AsnPlus::Stm32Bootloader
{
    class ArrayXor
    {
    public:
        static uint8_t calculate( const uint8_t * data, size_t length );
    };

    class BootloaderTransport
    {
    public:
        static constexpr uint8_t ACK                       = 0x79;
        static constexpr uint8_t NACK                      = 0x1F;
        static constexpr uint8_t BUSY                      = 0x76;

        static constexpr uint32_t MAX_TRANSPORT_TIMEOUT_MS = 200;
        static constexpr uint32_t MAX_BUSY_WAIT_MS         = 1000;

        enum class TransportType
        {
            I2C,
            SPI,
            UART
        };

        virtual bool synchronizeWithBootloader()                     = 0;
        virtual bool startCommand()                                  = 0;
        // virtual bool              readStatus( uint8_t & status )                  = 0;
        virtual bool sendData( const uint8_t * data, size_t length ) = 0;
        virtual bool receiveData( uint8_t * buffer, size_t length )  = 0;
        virtual bool waitAck()                                       = 0;

        // virtual bool useNoStretchCommands() const { return false; }
        virtual TransportType getTransportType() const               = 0;
    };

    // class SPIBootloaderTransport : public BootloaderTransport
    // {
    // public:
    //     SPIBootloaderTransport( Expander::SpiTransport & device ) : _spiDevice( device ) {}

    //     void initialize()
    //     {
    //         // Initialize SPI device with default parameters
    //     }

    //     bool synchronizeWithBootloader() override
    //     {
    //         // Nothing to be done for SPI
    //         uint8_t syncByte     = 0x5A;
    //         uint8_t receivedByte = 0;
    //         if ( ! _transfer( &syncByte, &receivedByte, 1 ) )
    //         {
    //             return false;
    //         }
    //         return waitAck();
    //     }

    //     bool startCommand() override
    //     {
    //         // Nothing to be done for SPI
    //         return _write( &SYNC_BYTE, 1 );
    //     }

    //     bool waitAck() override
    //     {
    //         uint8_t  sendBytes[ 2 ] = { 0x00, 0x01 };
    //         uint8_t  recvBytes[ 2 ] = { 0 };
    //         uint16_t attempts       = 10'000;
    //         while ( attempts-- > 0 )
    //         {
    //             if ( ! _transfer( sendBytes, recvBytes, 2 ) )
    //             {
    //                 return false;
    //             }
    //             uint8_t status = 0;
    //             if ( recvBytes[ 0 ] != DUMMY_BYTE )
    //             {
    //                 status = recvBytes[ 0 ];
    //             }
    //             else if ( recvBytes[ 1 ] != DUMMY_BYTE )
    //             {
    //                 status = recvBytes[ 1 ];
    //             }
    //             else
    //             {
    //                 continue;
    //             }
    //             if ( status == ACK )
    //             {
    //                 return _write( &status, 1 );    // Send ACK back
    //             }
    //             else if ( status == NACK )
    //             {
    //                 _write( &status, 1 );    // Send NACK back
    //                 return false;
    //             }
    //             else
    //             {
    //                 return false;
    //             }
    //         }
    //         return false;
    //     }

    //     bool sendData( const uint8_t * data, size_t length ) override { return _write( data, length ); }

    //     bool receiveData( uint8_t * buffer, size_t length ) override
    //     {
    //         uint8_t dummyReceive = 0;
    //         if ( ! _read( &dummyReceive, 1 ) ) return false;
    //         return _read( buffer, length );
    //     }

    //     TransportType getTransportType() const override { return TransportType::SPI; }

    // private:
    //     static constexpr uint8_t SYNC_BYTE  = 0x5A;
    //     static constexpr uint8_t DUMMY_BYTE = 0xA5;
    //     Expander::SpiTransport & _spiDevice;

    //     bool _write( const uint8_t * data, size_t length )
    //     {
    //         return _spiDevice.writeRead( data, NULL, length ) == ESP_OK;
    //     }

    //     bool _read( uint8_t * buffer, size_t length ) { return _spiDevice.writeRead( NULL, buffer, length ) ==
    //     ESP_OK; }

    //     bool _transfer( const uint8_t * write_buffer, uint8_t * read_buffer, size_t length )
    //     {
    //         return _spiDevice.writeRead( write_buffer, read_buffer, length ) == ESP_OK;
    //     }
    // };

    // class UARTTransport : public Transport
    // {
    // public:
    //     UARTTransport( Interfaces::Uart & uart ) : _uart( uart ) {}

    //     void initialize()
    //     {
    //         // Initialize UART device with default parameters
    //     }

    //     virtual bool synchronizeWithBootloader() override
    //     {
    //         _uart.flushRxBuffer();
    //         uint8_t syncByte     = SYNC_BYTE;
    //         uint8_t receivedByte = 0;
    //         if ( ! _uart.write( &syncByte, 1 ) )
    //         {
    //             ESP_LOGE( "UART", "Failed to send sync byte" );
    //             return false;
    //         }
    //         if ( ! _uart.read( &receivedByte, 1, 1000 ) )
    //         {
    //             ESP_LOGE( "UART", "Failed to receive sync ack byte" );
    //             return false;
    //         }
    //         ESP_LOGI( "UART", "Received sync ack byte: 0x%02X", receivedByte );
    //         return ( receivedByte == SYNC_ACK_BYTE );
    //     }

    //     virtual bool startCommand() override
    //     {
    //         // Nothing to be done for UART
    //         return true;
    //     }

    //     // virtual bool              readStatus( uint8_t & status )                  = 0;
    //     virtual bool sendData( const uint8_t * data, size_t length ) override { return _uart.write( data, length ); }

    //     virtual bool receiveData( uint8_t * buffer, size_t length ) override
    //     {
    //         return _uart.read( buffer, length, MAX_TRANSPORT_TIMEOUT_MS );
    //     }

    //     virtual bool waitAck() override
    //     {
    //         uint8_t status = 0;
    //         if ( ! _uart.read( &status, 1, MAX_TRANSPORT_TIMEOUT_MS ) )
    //         {
    //             return false;
    //         }
    //         return ( status == ACK );
    //     }

    //     TransportType getTransportType() const override { return TransportType::UART; }

    // private:
    //     static constexpr uint8_t SYNC_BYTE     = 0x7F;
    //     static constexpr uint8_t SYNC_ACK_BYTE = 0x79;

    //     Interfaces::Uart & _uart;
    // };
}    // namespace AsnPlus::Stm32Bootloader

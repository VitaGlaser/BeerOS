#pragma once

#include "transport.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <string.h>

namespace AsnPlus::Stm32Bootloader
{

    static constexpr uint8_t CMD_GET                     = 0x00;
    static constexpr uint8_t CMD_GET_VERSION             = 0x01;
    static constexpr uint8_t CMD_GET_ID                  = 0x02;
    static constexpr uint8_t CMD_READ_MEMORY             = 0x11;
    static constexpr uint8_t CMD_GO                      = 0x21;
    static constexpr uint8_t CMD_WRITE_MEMORY            = 0x31;
    static constexpr uint8_t CMD_NO_STRETCH_WRITE_MEMORY = 0x32;
    static constexpr uint8_t CMD_ERASE                   = 0x44;
    static constexpr uint8_t CMD_NO_STRETCH_ERASE        = 0x45;

    class Bootloader
    {
    public:
        enum class EraseType
        {
            FullErase,
            Bank1,
            Bank2,
        };

        Bootloader( BootloaderTransport & transport );

        bool synchronizeWithBootloader();
        bool readAddress( uint32_t address, uint8_t * data, uint16_t length );
        bool writeAddress( uint32_t address, const uint8_t * data, uint16_t length );
        bool eraseSpecial( uint16_t address );
        bool eraseSpecial( EraseType type );

    private:
        static constexpr uint16_t INTERNAL_BUFFER_SIZE = 1024;

        BootloaderTransport & _transport;

        uint8_t  _internalSendBuffer[ INTERNAL_BUFFER_SIZE + 1 ];
        uint16_t _internalSendBufferCounter = 0;

        bool _command( uint8_t command );
        bool _waitAck();

        template< typename T >
        void _addValue( T value )
        {
            constexpr size_t N = sizeof( T );

            for ( size_t i = 0; i < N; ++i )
            {
                _internalSendBuffer[ _internalSendBufferCounter + i ] =
                    static_cast< uint8_t >( ( value >> ( 8 * ( N - 1 - i ) ) ) & 0xFF );
            }
            _internalSendBufferCounter += N;
        }

        void    addData( uint8_t * data, uint16_t length );
        bool    _sendAndAck();
        uint8_t _cmdWriteId();
        uint8_t _cmdEraseId();
    };
}    // namespace AsnPlus::Stm32Bootloader

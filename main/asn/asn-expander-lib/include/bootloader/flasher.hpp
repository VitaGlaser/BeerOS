#pragma once

#include "bootloader.hpp"
#include "transport.hpp"
#include <cinttypes>
#include <cstring>
#include <iterator>
#include <stdint.h>

namespace AsnPlus::Stm32Bootloader
{
    class GenericFlasher
    {
    public:
        GenericFlasher(
            BootloaderTransport & transport,
            size_t                flashSize    = 32'768,
            uint32_t              flashAddress = 0x08'00'00'00
        );

        bool flashFirmware( const uint8_t * data, size_t size, bool noErase = false );
        bool verifyFirmware( const uint8_t * data, size_t size );
        bool exitBootloader();

        Bootloader & getBootloaderHandle();

    private:
        static constexpr size_t   MAX_UPLOAD_CHUNK_SIZE = 256;
        static constexpr uint32_t OPTR_BYTES_ADDR       = 0x1F'FF'78'00;
        static constexpr uint32_t BIT_NBOOT0            = 26;

        Bootloader _bootloader;

        size_t   _flashSize;
        uint32_t _flashAddress;

        void _setBit( uint32_t & value, uint8_t bit );
        void _resetBit( uint32_t & value, uint8_t bit );
        bool _getBit( uint32_t value, uint8_t bit );

        static uint32_t arrayToUint32( const uint8_t arr[ 4 ] );
        static void     uint32ToArray( uint32_t value, uint8_t arr[ 4 ] );
    };

}    // namespace AsnPlus::Stm32Bootloader

namespace AsnPlus::DeviceDefinitions
{
    constexpr size_t operator"" _KB( unsigned long long size )
    {
        // TODO (DK): Change this to a const
        return size * 1024;
    }

    constexpr size_t operator"" _MB( unsigned long long size )
    {
        // TODO (DK): Change this to a const
        return size * 1024 * 1024;
    }

    template< size_t flashSize, uint32_t flashAddress, uint8_t i2cAddress >
    class DeviceDefinition
    {
    public:
        static constexpr size_t   FlashSize    = flashSize;
        static constexpr uint32_t FlashAddress = flashAddress;
        static constexpr uint8_t  I2cAddress   = i2cAddress;
    };

    using STM32C051_32K = DeviceDefinition< 32_KB, 0x08'00'00'00, 0x76 >;
    using STM32C051_64K = DeviceDefinition< 64_KB, 0x08'00'00'00, 0x76 >;

}    // namespace AsnPlus::DeviceDefinitions

#pragma once

#include "asn/asn-core/logger.hpp"
#include "asn/asn-hal/include/peripherals/spi_master.hpp"
#include "transport.hpp"

namespace AsnPlus::Expander
{
    class SpiTransport : public Transport
    {
    public:
        static constexpr uint8_t DEFAULT_SLAVE_ID = 0;

        SpiTransport( ISpiMaster & master, uint8_t slaveId = DEFAULT_SLAVE_ID );

        void initialize();

        bool readRegister( uint16_t address, uint16_t & value ) override;
        bool readRegisters( uint16_t startAddress, uint16_t * data, size_t count ) override;
        bool writeRegister( uint16_t address, uint16_t data ) override;
        bool writeRegisters( uint16_t startAddress, const uint16_t * data, size_t count ) override;

        bool synchronizeWithBootlaoder() override;

        bool startCommand() override;
        bool sendData( const uint8_t * data, size_t length ) override;
        bool receiveData( uint8_t * buffer, size_t length ) override;
        bool waitAck() override;

    private:
        static constexpr const char TAG[]    = "SpiTransport";
        using Log                            = Logger< 0, TAG >;

        static constexpr uint32_t TIMEOUT_MS  = 500;
        static constexpr uint8_t  SOF         = 0x5A;
        static constexpr uint8_t  ACK         = 0x79;
        static constexpr uint8_t  NACK        = 0x1F;
        static constexpr uint8_t  DUMMY_BYTE  = 0xA5;

        ISpiMaster & _master;
        uint8_t      _slaveId;

        bool writeRead(
            const uint8_t * writeBuffer,
            uint8_t *       readBuffer,
            size_t          length,
            uint32_t        delayMs = TIMEOUT_MS
        );
    };
}    // namespace AsnPlus::Expander

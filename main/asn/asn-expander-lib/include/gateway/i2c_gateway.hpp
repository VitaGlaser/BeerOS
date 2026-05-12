#pragma once

#include "../register_addresses.hpp"
#include "registers.hpp"
#include <cstdint>

namespace AsnPlus::Expander
{
    class I2CGateway
    {
    public:
        I2CGateway( RegisterContext & context, uint16_t baseAddress );

        void poll();
        void enable();

        void sendTransaction();
        void sendTransaction(
            uint8_t         address,
            const uint8_t * txData,
            uint8_t         txLength,
            uint8_t *       rxData,
            uint8_t         rxLength
        );

    private:
        RegisterContext & _context;
        uint16_t          _baseAddress;

        I2CCTRL  ctrl { _context, _baseAddress, RegisterAddresses::I2Cx::I2CCTRL };
        I2CTRANS trans { _context, _baseAddress, RegisterAddresses::I2Cx::I2CTRANS };

        void _sendData( const uint8_t * data, uint8_t length );
        void _pullData( uint8_t * buffer, uint8_t length );
    };
}    // namespace AsnPlus::Expander

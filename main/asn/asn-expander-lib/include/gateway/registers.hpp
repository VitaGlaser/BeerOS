#pragma once

#include "../u16_register.hpp"
#include <cstdint>

namespace AsnPlus::Expander
{
    class I2CCTRL : public U16Register
    {
    public:
        using U16Register::U16Register;    // inherit constructors

        bool isISREnabled() const { return ( value >> 1 ) & 0b1; }

        bool isEnabled() const { return value & 0b1; }

        bool isTransactionInProgress() const { return ( value >> 2 ) & 0b1; }

        bool isTransactionError() const { return ( value >> 3 ) & 0b1; }

        void setISREnabled( bool en ) { value = ( value & ~( 1 << 1 ) ) | ( static_cast< uint16_t >( en ) << 1 ); }

        void setEnabled( bool en ) { value = ( value & ~1 ) | static_cast< uint16_t >( en ); }

        void setTransactionInProgress( bool inProgress )
        {
            value = ( value & ~( 1 << 2 ) ) | ( static_cast< uint16_t >( inProgress ) << 2 );
        }
    };

    class I2CTRANS : public U16Register
    {
    public:
        using U16Register::U16Register;    // inherit constructors

        // 8 bit address
        uint8_t getAddress() const { return value & 0xFF; }

        // 4 bit transmit length
        uint8_t getTransmitLength() const { return ( value >> 8 ) & 0xF; }

        // 4 bit receive length
        uint8_t getReceiveLength() const { return ( value >> 12 ) & 0xF; }

        void setAddress( uint8_t addr )
        {
            value = ( value & ~( 0xFF << 0 ) ) | ( static_cast< uint16_t >( addr ) << 0 );
        }

        void setTransmitLength( uint8_t len )
        {
            value = ( value & ~( 0xF << 8 ) ) | ( static_cast< uint16_t >( len ) << 8 );
        }

        void setReceiveLength( uint8_t len )
        {
            value = ( value & ~( 0xF << 12 ) ) | ( static_cast< uint16_t >( len ) << 12 );
        }
    };

}    // namespace AsnPlus::Expander

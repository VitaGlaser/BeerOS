#pragma once

#include "../u16_register.hpp"
#include <cstdint>

namespace AsnPlus::Expander
{

    class SYSFW : public U16Register
    {
    public:
        using U16Register::U16Register;    // inherit constructors

        uint8_t getFirmwareMajor() const { return static_cast< uint8_t >( ( value >> 8 ) & 0xFF ); }

        uint8_t getFirmwareMinor() const { return static_cast< uint8_t >( value & 0xFF ); }
    };

    class SYSPATCH : public U16Register
    {
    public:
        using U16Register::U16Register;    // inherit constructors

        uint16_t getFirmwarePatch() const { return value; }
    };

    class SYSSIG : public U16Register
    {
    public:
        using U16Register::U16Register;    // inherit constructors

        uint16_t getSignatureByte() const { return value; }
    };

    class SYSCFGA : public U16Register
    {
    public:
        using U16Register::U16Register;    // inherit constructors

        void setBootloader() { value |= ( 1 << 15 ); }

        bool isBootloaderTriggered() const { return static_cast< bool >( ( value >> 15 ) & 0b1 ); }

        void setReset() { value |= ( 1 << 14 ); }

        bool isResetTriggered() const { return static_cast< bool >( ( value >> 14 ) & 0b1 ); }

        void setShutdown() { value |= ( 1 << 13 ); }

        bool isShutdownTriggered() const { return static_cast< bool >( ( value >> 13 ) & 0b1 ); }

        bool isISRResetEnabled() const { return static_cast< bool >( ( value >> 1 ) & 0b1 ); }

        void setISRReset( bool enable )
        {
            if ( enable )
                value |= ( 1 << 1 );
            else
                value &= ~( 1 << 1 );
        }

        bool isExternalISREnabled() const { return static_cast< bool >( value & 0b1 ); }

        void setExternalISR( bool enable )
        {
            if ( enable )
                value |= 0b1;
            else
                value &= ~0b1;
        }
    };

    class IOxISRFLG : public U16Register
    {
    public:
        using U16Register::U16Register;    // inherit constructors

        bool getFlag( uint8_t pin ) const { return static_cast< bool >( ( value >> pin ) & 0b1 ); }

        void setFlag( uint8_t pin, bool flag )
        {
            if ( flag )
                value |= ( 1 << pin );
            else
                value &= ~( 1 << pin );
        }

        void clearFlag( uint8_t pin )
        {
            value |= ( 1 << pin );    // Write 1 to clear
        }

        void clearAllFlags()
        {
            value = 0xFFFF;    // Write 1 to all bits to clear them
        }

        uint16_t getAllFlags() const { return value; }
    };

    class ADCxISRFLG : public U16Register
    {
    public:
        using U16Register::U16Register;    // inherit constructors

        // Buffer flags (bits 15:8)
        bool getBufferFlag( uint8_t channel ) const
        {
            return static_cast< bool >( ( value >> ( channel + 8 ) ) & 0b1 );
        }

        void setBufferFlag( uint8_t channel, bool flag )
        {
            if ( flag )
                value |= ( 1 << ( channel + 8 ) );
            else
                value &= ~( 1 << ( channel + 8 ) );
        }

        void clearBufferFlag( uint8_t channel )
        {
            value |= ( 1 << ( channel + 8 ) );    // Write 1 to clear
        }

        // ADC measurement flags (bits 7:0)
        bool getADCFlag( uint8_t channel ) const { return static_cast< bool >( ( value >> channel ) & 0b1 ); }

        void setADCFlag( uint8_t channel, bool flag )
        {
            if ( flag )
                value |= ( 1 << channel );
            else
                value &= ~( 1 << channel );
        }

        void clearADCFlag( uint8_t channel )
        {
            value |= ( 1 << channel );    // Write 1 to clear
        }

        void clearAllFlags()
        {
            value = 0xFFFF;    // Write 1 to all bits to clear them
        }

        uint16_t getAllFlags() const { return value; }
    };

}    // namespace AsnPlus::Expander

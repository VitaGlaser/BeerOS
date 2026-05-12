#pragma once

#include "../u16_register.hpp"
#include <cstdint>

namespace AsnPlus::Expander::Gpio
{
    /*
    Bit:     15    14    13    12    11    10     9     8     7     6     5     4     3     2     1     0
           +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
    Field: |      FILTERB[3:0]     |      FILTERA[3:0]     | SAMPLING[4:0]               |  X  |ISREN|ENABLE|
    Access: R/W   R/W   R/W   R/W   R/W   R/W   R/W   R/W   R/W   R/W   R/W   R/W   R/W     -    R/W    R/W
    Reset:   0     0     0     0     0     0     0     0     0     0     0     0     0      -     0      0
    */
    class IOxCTRL : public U16Register
    {
    public:
        using U16Register::U16Register;    // inherit constructors

        enum class FilterType : uint8_t
        {
            FT4  = 0x1,
            FT8  = 0x2,
            FT16 = 0x3,
            FT32 = 0x4,
        };
        enum class SamplingTime : uint8_t
        {
            SPEED_ALWAYS = 0x0,
            SPEED_10US   = 0x1,
            SPEED_100US  = 0x2,
            SPEED_500US  = 0x3,
            SPEED_1MS    = 0x4,
            SPEED_10MS   = 0x5,
            SPEED_100MS  = 0x6,
            SPEED_500MS  = 0x7,
        };

        // getters
        FilterType getFilterTypeA() const { return static_cast< FilterType >( ( value >> 8 ) & 0b1111 ); }

        FilterType getFilterTypeB() const { return static_cast< FilterType >( ( value >> 12 ) & 0b1111 ); }

        SamplingTime getSamplingTime() const { return static_cast< SamplingTime >( ( value >> 3 ) & 0b11111 ); }

        bool isISREnabled() const { return ( value >> 1 ) & 0b1; }

        bool isEnabled() const { return value & 0b1; }

        // setters
        void setFilterTypeA( FilterType f )
        {
            value = ( value & ~( 0b1111 << 8 ) ) | ( static_cast< uint16_t >( f ) << 8 );
        }

        void setFilterTypeB( FilterType f )
        {
            value = ( value & ~( 0b1111 << 12 ) ) | ( static_cast< uint16_t >( f ) << 12 );
        }

        void setSamplingTime( SamplingTime t )
        {
            value = ( value & ~( 0b11111 << 3 ) ) | ( static_cast< uint16_t >( t ) << 3 );
        }

        void setISREnabled( bool en ) { value = ( value & ~( 1 << 1 ) ) | ( static_cast< uint16_t >( en ) << 1 ); }

        void setEnabled( bool en ) { value = ( value & ~1 ) | static_cast< uint16_t >( en ); }
    };

    class IOxMODE : public U16Register
    {
    public:
        using U16Register::U16Register;

        enum class Mode : uint8_t
        {
            INPUT_HIGHZ    = 0x0,
            INPUT_PULLUP   = 0x1,
            INPUT_PULLDOWN = 0x2,
            OUTPUT         = 0x3,
        };

        Mode getMode( uint8_t pin ) const { return static_cast< Mode >( ( value >> ( pin * 2 ) ) & 0b11 ); }

        void setMode( uint8_t pin, Mode m )
        {
            value = ( value & ~( 0b11 << ( pin * 2 ) ) ) | ( static_cast< uint16_t >( m ) << ( pin * 2 ) );
        }
    };

    class IOxFLT : public U16Register
    {
    public:
        using U16Register::U16Register;

        enum class FilterSelect : uint8_t
        {
            FLT_OFF  = 0x0,
            FLT_A    = 0x1,
            FLT_B    = 0x2,
            RESERVED = 0x3,
        };

        FilterSelect getFilter( uint8_t pin ) const
        {
            return static_cast< FilterSelect >( ( value >> ( pin * 2 ) ) & 0b11 );
        }

        void setFilter( uint8_t pin, FilterSelect f )
        {
            value = ( value & ~( 0b11 << ( pin * 2 ) ) ) | ( static_cast< uint16_t >( f ) << ( pin * 2 ) );
        }
    };

    class IOxISRMSK : public U16Register
    {
    public:
        using U16Register::U16Register;

        enum class ISRMode : uint8_t
        {
            ISR_OFF     = 0x0,
            ISR_RISING  = 0x1,
            ISR_FALLING = 0x2,
            ISR_TOGGLE  = 0x3,
        };

        ISRMode getISRMode( uint8_t pin ) const { return static_cast< ISRMode >( ( value >> ( pin * 2 ) ) & 0b11 ); }

        void setISRMode( uint8_t pin, ISRMode m )
        {
            value = ( value & ~( 0b11 << ( pin * 2 ) ) ) | ( static_cast< uint16_t >( m ) << ( pin * 2 ) );
        }
    };

    /**
     * @brief Class representing an INPUT register.
     */
    class IOxIN : public U16Register
    {
    public:
        using U16Register::U16Register;

        bool getPinValue( uint8_t pin ) const { return ( value >> pin ) & 0b1; }

        uint16_t getReadoutValue() const { return value; }

        void setSinglePinReadoutValue( uint8_t pin, bool newValue )
        {
            if ( newValue )
                value |= ( 1 << pin );
            else
                value &= ~( 1 << pin );
        }

        void setReadoutValue( uint16_t newValue ) { value = newValue; }
    };

    /**
     * @brief Class representing an OUTPUT register.
     */
    class IOxOUT : public U16Register
    {
    public:
        using U16Register::U16Register;

        bool getOutputValue( uint8_t pin ) const { return ( value >> pin ) & 0b1; }

        uint16_t getAllOutputValues() const { return value; }

        void setOutputValue( uint8_t pin, bool newValue )
        {
            if ( newValue )
                value |= ( 1 << pin );
            else
                value &= ~( 1 << pin );
        }

        void setAllOutputValues( uint16_t newValue ) { value = newValue; }
    };

}    // namespace AsnPlus::Expander::Gpio

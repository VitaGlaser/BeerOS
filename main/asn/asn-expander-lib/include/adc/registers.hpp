#pragma once

#include "../u16_register.hpp"
#include <cstdint>

namespace AsnPlus::Expander::Adc
{
    class ADCCTRL : public U16Register
    {
    public:
        using U16Register::U16Register;

        enum class SamplingFrequency : uint8_t
        {
            SMP1  = 0x0,    // 1 kHz
            SMP2  = 0x1,    // 2 kHz
            SMP5  = 0x2,    // 4 kHz
            SMP8  = 0x3,    // 8 kHz
            SMP16 = 0x4,    // 16 kHz
            SMP24 = 0x5,    // 24 kHz
            SMP32 = 0x6,    // 32 kHz
            SMP48 = 0x7,    // 48 kHz
        };

        enum class TriggerSource : uint8_t
        {
            INT  = 0x0,    // Internal timer
            EXTR = 0x1,    // External rising edge
            EXTF = 0x2,    // External falling edge
            EXTT = 0x3,    // External toggle
        };

        void setCalibration( bool enable )
        {
            if ( enable )
                value |= ( 1 << 15 );
            else
                value &= ~( 1 << 15 );
        }

        bool isCalibrating() const { return static_cast< bool >( ( value >> 15 ) & 0b1 ); }

        SamplingFrequency getSamplingFrequency() const
        {
            return static_cast< SamplingFrequency >( ( value >> 5 ) & 0b111 );
        }

        void setSamplingFrequency( SamplingFrequency freq )
        {
            value &= ~( 0b111 << 5 );    // Clear bits
            value |= ( static_cast< uint16_t >( freq ) << 5 );
        }

        TriggerSource getTriggerSource() const { return static_cast< TriggerSource >( ( value >> 2 ) & 0b11 ); }

        void setTriggerSource( TriggerSource source )
        {
            value &= ~( 0b11 << 2 );    // Clear bits
            value |= ( static_cast< uint16_t >( source ) << 2 );
        }

        bool isISREnabled() const { return static_cast< bool >( ( value >> 1 ) & 0b1 ); }

        void setISREnabled( bool enable )
        {
            if ( enable )
                value |= ( 1 << 1 );
            else
                value &= ~( 1 << 1 );
        }

        bool isEnabled() const { return static_cast< bool >( value & 0b1 ); }

        void setEnabled( bool enable )
        {
            if ( enable )
                value |= 0b1;
            else
                value &= ~0b1;
        }
    };

    class ADCCMPxx : public U16Register
    {
    public:
        using U16Register::U16Register;

        uint16_t getComparatorValue() const
        {
            return value & 0x0FFF;    // 12-bit value
        }

        void setComparatorValue( uint16_t value )
        {
            this->value = value & 0x0FFF;    // Ensure only 12 bits are used
        }
    };

    class ADCMxCFG : public U16Register
    {
    public:
        using U16Register::U16Register;

        enum class ISRMode : uint8_t
        {
            OFF          = 0x0,    // Interrupt disabled
            ON_OVERFLOW  = 0x1,    // Interrupt on upper limit exceeded
            ON_UNDERFLOW = 0x2,    // Interrupt on lower limit exceeded
            ON_EXIT      = 0x3,    // Interrupt on leaving range
            ON_ENTER     = 0x4,    // Interrupt on entering range
        };

        enum class ComparatorSelect : uint8_t
        {
            CMP0 = 0x0,    // Use ADCCMP0L and ADCCMP0H
            CMP1 = 0x1,    // Use ADCCMP1L and ADCCMP1H
        };

        enum class BufferConfig : uint8_t
        {
            SINGLE = 0x0,    // Single value buffer
            FIFO   = 0x1,    // FIFO buffer (64 samples)
            LIFO   = 0x2,    // LIFO buffer (64 samples)
        };

        enum class LPFrequency : uint8_t
        {
            OFF    = 0x0,    // Low pass filter disabled
            LPF1   = 0x1,    // 1 Hz
            LPF2   = 0x2,    // 2 Hz
            LPF5   = 0x3,    // 5 Hz
            LPF10  = 0x4,    // 10 Hz
            LPF20  = 0x5,    // 20 Hz
            LPF50  = 0x6,    // 50 Hz
            LPF100 = 0x7,    // 100 Hz
            LPF200 = 0x8,    // 200 Hz
            LPF500 = 0x9,    // 500 Hz
            LPF1K  = 0xA,    // 1 kHz
            LPF2K  = 0xB,    // 2 kHz
            LPF5K  = 0xC,    // 5 kHz
            LPF10K = 0xD,    // 10 kHz
            LPF20K = 0xE,    // 20 kHz
        };

        enum class ChannelSelect : uint8_t
        {
            OFF  = 0x0,    // Channel disabled
            CH0  = 0x1,    // ADC input 0
            CH1  = 0x2,    // ADC input 1
            CH2  = 0x3,    // ADC input 2
            CH3  = 0x4,    // ADC input 3
            CH4  = 0x5,    // ADC input 4
            CH5  = 0x6,    // ADC input 5
            CH6  = 0x7,    // ADC input 6
            CH7  = 0x8,    // ADC input 7
            CH8  = 0x9,    // ADC input 8
            CH9  = 0xA,    // ADC input 9
            CH10 = 0xB,    // ADC input 10
            CH11 = 0xC,    // ADC input 11
            CH12 = 0xD,    // ADC input 12
            VREF = 0xE,    // Voltage reference
            TEMP = 0xF,    // Internal temperature sensor
        };

        ISRMode getISRMode() const { return static_cast< ISRMode >( ( value >> 13 ) & 0b111 ); }

        void setISRMode( ISRMode mode )
        {
            value &= ~( 0b111 << 13 );    // Clear bits
            value |= ( static_cast< uint16_t >( mode ) << 13 );
        }

        bool isBufferISREnabled() const { return static_cast< bool >( ( value >> 12 ) & 0b1 ); }

        void setBufferISREnabled( bool enable )
        {
            if ( enable )
                value |= ( 1 << 12 );
            else
                value &= ~( 1 << 12 );
        }

        ComparatorSelect getComparatorSelect() const
        {
            return static_cast< ComparatorSelect >( ( value >> 10 ) & 0b11 );
        }

        void setComparatorSelect( ComparatorSelect select )
        {
            value &= ~( 0b11 << 10 );    // Clear bits
            value |= ( static_cast< uint16_t >( select ) << 10 );
        }

        BufferConfig getBufferConfig() const { return static_cast< BufferConfig >( ( value >> 8 ) & 0b11 ); }

        void setBufferConfig( BufferConfig config )
        {
            value &= ~( 0b11 << 8 );    // Clear bits
            value |= ( static_cast< uint16_t >( config ) << 8 );
        }

        LPFrequency getLPFrequency() const { return static_cast< LPFrequency >( ( value >> 4 ) & 0b1111 ); }

        void setLPFrequency( LPFrequency freq )
        {
            value &= ~( 0b1111 << 4 );    // Clear bits
            value |= ( static_cast< uint16_t >( freq ) << 4 );
        }

        ChannelSelect getChannelSelect() const { return static_cast< ChannelSelect >( value & 0b1111 ); }

        void setChannelSelect( ChannelSelect channel )
        {
            value &= ~0b1111;    // Clear bits
            value |= static_cast< uint16_t >( channel );
        }
    };

    class ADCxBUF : public U16Register
    {
    public:
        using U16Register::U16Register;

        bool isEmpty() const { return static_cast< bool >( ( value >> 15 ) & 0b1 ); }

        bool isOverflow() const { return static_cast< bool >( ( value >> 14 ) & 0b1 ); }

        uint16_t getBufferValue() const
        {
            return value & 0x3FFF;    // 14-bit value
        }
    };

}    // namespace AsnPlus::Expander::Adc

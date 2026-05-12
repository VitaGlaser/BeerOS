#pragma once

#include "../u16_register.hpp"
#include <cstdint>

namespace AsnPlus::Expander
{
    class TIMxCR : public U16Register
    {
    public:
        using U16Register::U16Register;

        enum class Mode : uint8_t
        {
            SQR   = 0x0,    // Symmetric rectangular (toggle/OC)
            PWM   = 0x1,    // PWM generation
            FREQ  = 0x2,    // Frequency measurement
            COUNT = 0x3,    // Input capture / counting
        };

        enum class ClockDiv : uint8_t
        {
            DIV1 = 0x0,    // tDTS = tTIM
            DIV2 = 0x1,    // tDTS = 2 * tTIM
            DIV4 = 0x2,    // tDTS = 4 * tTIM
        };

        enum class CenterAlign : uint8_t
        {
            EDGE    = 0x0,    // Edge-aligned, direction set by DIR bit
            CENTER1 = 0x1,
            CENTER2 = 0x2,
            CENTER3 = 0x3,
        };

        bool isEnabled() const { return value & 0x0001; }

        bool isDir() const { return value & 0x0002; }    // 0=up, 1=down

        ClockDiv getClockDivision() const { return static_cast< ClockDiv >( ( value >> 2 ) & 0x03 ); }

        CenterAlign getCenterAlignMode() const { return static_cast< CenterAlign >( ( value >> 4 ) & 0x03 ); }

        Mode getMode() const { return static_cast< Mode >( ( value >> 6 ) & 0x03 ); }

        bool isAutoReloadPreloadEnabled() const { return value & 0x8000; }

        void setEnabled( bool en ) { setBits( 0, 1, en ); }

        void setDir( bool down ) { setBits( 1, 1, down ); }

        void setClockDivision( ClockDiv div ) { setBits( 2, 0x03, static_cast< uint16_t >( div ) ); }

        void setCenterAlignMode( CenterAlign mode ) { setBits( 4, 0x03, static_cast< uint16_t >( mode ) ); }

        void setMode( Mode mode ) { setBits( 6, 0x03, static_cast< uint16_t >( mode ) ); }

        void setAutoReloadPreload( bool en ) { en ? ( value |= 0x8000 ) : ( value &= ~0x8000 ); }

    private:
        void setBits( uint8_t shift, uint16_t mask, uint16_t val )
        {
            value = ( value & ~( mask << shift ) ) | ( ( val & mask ) << shift );
        }
    };

    class TIMxCCMR : public U16Register
    {
    public:
        using U16Register::U16Register;

        enum class Polarity : uint8_t
        {
            ACTIVE_HIGH = 0x0,    // Output: active high  / Input: rising edge
            ACTIVE_LOW  = 0x1,    // Output: active low   / Input: falling edge
            BOTH_EDGES  = 0x2,    // Input capture only
        };

        enum class ICPrescaler : uint8_t
        {
            EVERY_EDGE = 0x0,
            EVERY_2ND  = 0x1,
            EVERY_4TH  = 0x2,
            EVERY_8TH  = 0x3,
        };

        bool isChannelEnabled() const { return value & 0x8000; }

        Polarity getPolarity() const { return static_cast< Polarity >( ( value >> 6 ) & 0x03 ); }

        ICPrescaler getICPrescaler() const { return static_cast< ICPrescaler >( ( value >> 4 ) & 0x03 ); }

        uint8_t getICFilter() const
        {
            return static_cast< uint8_t >( value & 0x0F );
        }    // 0=off, 1–15=increasing filter strength

        void setChannelEnabled( bool en ) { en ? ( value |= 0x8000 ) : ( value &= ~0x8000 ); }

        void setPolarity( Polarity pol ) { setBits( 6, 0x03, static_cast< uint16_t >( pol ) ); }

        void setICPrescaler( ICPrescaler psc ) { setBits( 4, 0x03, static_cast< uint16_t >( psc ) ); }

        void setICFilter( uint8_t filter ) { setBits( 0, 0x0F, filter ); }

    private:
        void setBits( uint8_t shift, uint16_t mask, uint16_t val )
        {
            value = ( value & ~( mask << shift ) ) | ( ( val & mask ) << shift );
        }
    };

    class TIMxIER : public U16Register
    {
    public:
        using U16Register::U16Register;

        bool isUpdateIE() const { return value & 0x01; }

        bool isCaptureCompareIE( uint8_t ch ) const { return value & ( 1 << ( ch + 1 ) ); }
    };

    class TIMxPSC : public U16Register
    {
    public:
        using U16Register::U16Register;

        uint16_t getValue() const { return value; }

        void setValue( uint16_t val ) { value = val; }
    };

    class TIMxCNT : public U16Register
    {
    public:
        using U16Register::U16Register;

        uint16_t getValue() const { return value; }

        void setValue( uint16_t val ) { value = val; }
    };

    class TIMxARR : public U16Register
    {
    public:
        using U16Register::U16Register;

        uint16_t getValue() const { return value; }

        void setValue( uint16_t val ) { value = val; }
    };

    class TIMxCCR : public U16Register
    {
    public:
        using U16Register::U16Register;

        uint16_t getValue() const { return value; }

        void setValue( uint16_t val ) { value = val; }
    };
}    // namespace AsnPlus::Expander

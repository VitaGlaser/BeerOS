#pragma once

// TODO(DK): TEST PWM

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "../register_addresses.hpp"
#include "../u16_register.hpp"

#include "registers.hpp"

namespace AsnPlus::Expander::Timers
{
    class Timer
    {
    public:
        class Channel
        {
        public:
            Channel( Timer & timer, uint8_t index );

            void enable();
            void disable();
            void setEnabled( bool enabled );

            uint16_t getValue();
            float    getFrequency();
            float    getDutyCycle();
            void     setDutyCycle( float pct );
            void     setValue( uint16_t val );
            void     setPolarity( TIMxCCMR::Polarity pol );
            void     setICPrescaler( TIMxCCMR::ICPrescaler psc );
            void     setICFilter( uint8_t filter );
            TIMxCCMR & ccmr();
            TIMxCCR &  ccr();

        private:
            Timer & _timer;
            uint8_t _index;    // 0-based
        };

        Timer( RegisterContext & context, uint16_t baseAddress );

        void poll();
        void enable();
        void disable();
        void setMode( TIMxCR::Mode mode );
        void setClockDivision( TIMxCR::ClockDiv div );
        void setCenterAlignMode( TIMxCR::CenterAlign mode );
        void setDir( bool down );
        void setAutoReloadPreload( bool en );
        void setPrescaler( uint16_t prescaler );
        void setAutoReload( uint16_t autoReload );

        // Channel number follows schematic indexing (1–4)
        Channel channel( uint8_t chNumber );

        TIMxCR  CR;
        TIMxIER IER;
        TIMxPSC PSC;
        TIMxCNT CNT;
        TIMxARR ARR;

        TIMxCCMR CCMR1;
        TIMxCCMR CCMR2;
        TIMxCCMR CCMR3;
        TIMxCCMR CCMR4;

        TIMxCCR CCR1;
        TIMxCCR CCR2;
        TIMxCCR CCR3;
        TIMxCCR CCR4;

        TIMxCCMR * _ccmr[ 4 ] = { &CCMR1, &CCMR2, &CCMR3, &CCMR4 };
        TIMxCCR *  _ccr[ 4 ]  = { &CCR1, &CCR2, &CCR3, &CCR4 };

    private:
        static constexpr const char TAG[] = "Expander::Timer";
        using Log                         = Logger< 0, TAG >;
    };
}    // namespace AsnPlus::Expander::Timers

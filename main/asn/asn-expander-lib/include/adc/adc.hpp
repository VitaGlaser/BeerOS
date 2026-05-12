#pragma once

#include "../register_addresses.hpp"
#include "asn/asn-core/timer.hpp"
#include "registers.hpp"
#include <cstdint>

namespace AsnPlus::Expander::Adc
{
    class Adc
    {
    public:
        class Channel
        {
        public:
            Channel( Adc & adc, uint8_t channelNumber );

            void setPhysicalChannel( ADCMxCFG::ChannelSelect channelSelect );
            void setFilter( ADCMxCFG::LPFrequency freq );

            uint16_t getValue();
            void     reload();

        private:
            Adc &   _adc;
            uint8_t _channelNumber;
        };

        Adc( RegisterContext & context, uint16_t baseAddress );

        void poll();
        void setSamplingFrequency( ADCCTRL::SamplingFrequency freq );
        void startCalibration();
        bool isCalibrating();
        bool waitCalibration();
        void setEnabled( bool enabled );
        void enable();
        void disable();

        Channel channel( uint8_t chNumber );

        void readoutLoop();

    private:
        RegisterContext & _context;
        uint16_t          _baseAddress;

        using Addr = RegisterAddresses::ADCx;
        ADCCTRL  ctrl { _context, _baseAddress, Addr::ADCCTRL };
        ADCMxCFG cfg1 { _context, _baseAddress, Addr::ADCM1CFG };
        ADCMxCFG cfg2 { _context, _baseAddress, Addr::ADCM2CFG };
        ADCMxCFG cfg3 { _context, _baseAddress, Addr::ADCM3CFG };
        ADCMxCFG cfg4 { _context, _baseAddress, Addr::ADCM4CFG };
        ADCMxCFG cfg5 { _context, _baseAddress, Addr::ADCM5CFG };
        ADCMxCFG cfg6 { _context, _baseAddress, Addr::ADCM6CFG };
        ADCMxCFG cfg7 { _context, _baseAddress, Addr::ADCM7CFG };
        ADCMxCFG cfg8 { _context, _baseAddress, Addr::ADCM8CFG };
        ADCMxCFG cfg9 { _context, _baseAddress, Addr::ADCM9CFG };
        ADCMxCFG cfg10 { _context, _baseAddress, Addr::ADCM10CFG };
        ADCMxCFG cfg11 { _context, _baseAddress, Addr::ADCM11CFG };
        ADCMxCFG cfg12 { _context, _baseAddress, Addr::ADCM12CFG };

        ADCMxCFG * _cfgs[ 12 ] =
            { &cfg1, &cfg2, &cfg3, &cfg4, &cfg5, &cfg6, &cfg7, &cfg8, &cfg9, &cfg10, &cfg11, &cfg12 };

    public:
        ADCxBUF buf1 { _context, _baseAddress, Addr::ADCBUF1 };
        ADCxBUF buf2 { _context, _baseAddress, Addr::ADCBUF2 };
        ADCxBUF buf3 { _context, _baseAddress, Addr::ADCBUF3 };
        ADCxBUF buf4 { _context, _baseAddress, Addr::ADCBUF4 };
        ADCxBUF buf5 { _context, _baseAddress, Addr::ADCBUF5 };
        ADCxBUF buf6 { _context, _baseAddress, Addr::ADCBUF6 };
        ADCxBUF buf7 { _context, _baseAddress, Addr::ADCBUF7 };
        ADCxBUF buf8 { _context, _baseAddress, Addr::ADCBUF8 };
        ADCxBUF buf9 { _context, _baseAddress, Addr::ADCBUF9 };
        ADCxBUF buf10 { _context, _baseAddress, Addr::ADCBUF10 };
        ADCxBUF buf11 { _context, _baseAddress, Addr::ADCBUF11 };
        ADCxBUF buf12 { _context, _baseAddress, Addr::ADCBUF12 };

        ADCxBUF * _bufs[ 12 ] =
            { &buf1, &buf2, &buf3, &buf4, &buf5, &buf6, &buf7, &buf8, &buf9, &buf10, &buf11, &buf12 };
    };
}    // namespace AsnPlus::Expander::Adc

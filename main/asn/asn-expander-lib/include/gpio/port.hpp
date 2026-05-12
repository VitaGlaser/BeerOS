#pragma once

#include "../register_addresses.hpp"
#include "registers.hpp"

namespace AsnPlus::Expander
{
    using PinMode = Gpio::IOxMODE::Mode;
}

namespace AsnPlus::Expander::Gpio
{

    class Port
    {
    public:
        class Pin
        {
        public:
            Pin( Port & port, uint8_t pinNumber );

            void setMode( IOxMODE::Mode mode );
            IOxMODE::Mode getMode();

            void setOutput( bool out );
            bool getCurrentOutput();

            void set();
            void reset();
            bool read();

            void             setFilterType( IOxFLT::FilterSelect select );
            IOxFLT::FilterSelect getFilterType();

            void               setISREnabled( IOxISRMSK::ISRMode mode );
            IOxISRMSK::ISRMode getISRMode();

            Port &  getPort() const;
            uint8_t getPinNumber() const;

        private:
            Port &  _port;
            uint8_t _pinNumber;
        };

        Port( RegisterContext & context, uint16_t baseAddress );

        ~Port()                          = default;

        Port( const Port & )             = delete;
        Port( Port && )                  = delete;
        Port & operator=( const Port & ) = delete;
        Port & operator=( Port && )      = delete;

        void poll();

        void setEnabled( bool en );
        void enable();
        void disable();

        void setISREnabled( bool en );
        void enableISR();
        void disableISR();

        void setFilterTypeA( IOxCTRL::FilterType filterType );
        void setFilterTypeB( IOxCTRL::FilterType filterType );
        void setSamplingTime( IOxCTRL::SamplingTime samplingTime );

        Pin  pin( uint8_t pinNumber );
        char getName() const;

    protected:
        RegisterContext & _context;
        uint16_t          _baseAddress;

        using Addr = RegisterAddresses::IOx;
        Gpio::IOxCTRL   ctrl { _context, _baseAddress, Addr::IOxCTRLA };
        Gpio::IOxMODE   modeA { _context, _baseAddress, Addr::IOxMODEA };
        Gpio::IOxMODE   modeB { _context, _baseAddress, Addr::IOxMODEB };
        Gpio::IOxFLT    fltA { _context, _baseAddress, Addr::IOxFLTA };
        Gpio::IOxFLT    fltB { _context, _baseAddress, Addr::IOxFLTB };
        Gpio::IOxISRMSK isrMaskA { _context, _baseAddress, Addr::IOxISRMSKA };
        Gpio::IOxISRMSK isrMaskB { _context, _baseAddress, Addr::IOxISRMSKB };
        Gpio::IOxIN     in { _context, _baseAddress, Addr::IOxIN };
        Gpio::IOxOUT    out { _context, _baseAddress, Addr::IOxOUT };
    };

}    // namespace AsnPlus::Expander::Gpio

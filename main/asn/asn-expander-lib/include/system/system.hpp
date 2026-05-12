#pragma once

#include "../register_addresses.hpp"
#include "registers.hpp"
#include <cstdint>

namespace AsnPlus::Expander
{
    class SystemPeripheral
    {
    public:
        SystemPeripheral( RegisterContext & context, uint16_t baseAddress );

        void     poll();
        void     loadInitial();
        void     getVersion( uint8_t & major, uint8_t & minor, uint16_t & patch );
        uint16_t getSignature();
        void     requestBootloader();
        void     reset();
        void     shutdown();

    private:
        RegisterContext & _context;
        uint16_t          _baseAddress;

        SYSFW      _fw { _context, _baseAddress, RegisterAddresses::SYSTEM::SYSFW };
        SYSPATCH   _patch { _context, _baseAddress, RegisterAddresses::SYSTEM::SYSPATCH };
        SYSSIG     _sigMSB { _context, _baseAddress, RegisterAddresses::SYSTEM::SYSSIGNHI };
        SYSSIG     _sigLSB { _context, _baseAddress, RegisterAddresses::SYSTEM::SYSSIGNLOW };
        SYSCFGA    _cfga { _context, _baseAddress, RegisterAddresses::SYSTEM::SYSCFGA };
        IOxISRFLG  _ioa_isr_flg { _context, _baseAddress, RegisterAddresses::SYSTEM::IOAISRFLG };
        IOxISRFLG  _iob_isr_flg { _context, _baseAddress, RegisterAddresses::SYSTEM::IOBISRFLG };
        ADCxISRFLG _adc_isr_flg { _context, _baseAddress, RegisterAddresses::SYSTEM::ADCISRFLG };
    };
}    // namespace AsnPlus::Expander

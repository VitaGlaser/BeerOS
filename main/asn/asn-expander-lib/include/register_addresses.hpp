#pragma once

#include <cstdint>

namespace AsnPlus::Expander
{
    static constexpr uint32_t EXPANDER_CLOCK_HZ = 48'000'000;
}

namespace AsnPlus
{

    struct RegisterAddresses
    {
        enum Peripherals : uint16_t
        {
            PER_SYSTEM = 0x0001,    // Starts at 0x0001
            PER_IOA    = 0x000E,    // Starts at 0x000E
            PER_IOB    = 0x0017,    // Starts at 0x0017
            PER_ADC    = 0x0020,    // Starts at 0x0020
            PER_TIMERA = 0x003D,    // Starts at 0x003D
            PER_TIMERB = 0x004A,    // Starts at 0x004A
            PER_TIMERC = 0x0057,    // Starts at 0x0057
            PER_TIMERD = 0x0064,    // Starts at 0x0064
            PER_TIMERE = 0x0071,    // Starts at 0x0071
            PER_I2C    = 0x007E,    // Starts at 0x007E
        };

        enum SYSTEM : uint8_t
        {
            SYSFW      = 0x00,    // Base + 0x00 (0x0001)
            SYSPATCH   = 0x01,    // Base + 0x01 (0x0002)
            SYSSIGNHI  = 0x02,    // Base + 0x02 (0x0003)
            SYSSIGNLOW = 0x03,    // Base + 0x03 (0x0004)
            SYSCFGA    = 0x04,    // Base + 0x04 (0x0005)
            IOAISRFLG  = 0x05,    // Base + 0x05 (0x0006)
            IOBISRFLG  = 0x06,    // Base + 0x06 (0x0007)
            ADCISRFLG  = 0x07,    // Base + 0x07 (0x0008)
            TIMAISRFLG = 0x08,    // Base + 0x08 (0x0009)
            TIMBISRFLG = 0x09,    // Base + 0x09 (0x000A)
            TIMCISRFLG = 0x0A,    // Base + 0x0A (0x000B)
            TIMDISRFLG = 0x0B,    // Base + 0x0B (0x000C)
            TIMEISRFLG = 0x0C,    // Base + 0x0C (0x000D)
        };

        enum IOx : uint8_t
        {
            IOxCTRLA   = 0x00,
            IOxMODEA   = 0x01,
            IOxMODEB   = 0x02,
            IOxFLTA    = 0x03,
            IOxFLTB    = 0x04,
            IOxISRMSKA = 0x05,
            IOxISRMSKB = 0x06,
            IOxIN      = 0x07,
            IOxOUT     = 0x08,
        };

        enum ADCx : uint8_t
        {
            ADCCTRL   = 0x00,    // Base + 0x00 (0x0020)
            ADCCMP1H  = 0x01,    // Base + 0x01 (0x0021)
            ADCCMP1L  = 0x02,    // Base + 0x02 (0x0022)
            ADCCMP2H  = 0x03,    // Base + 0x03 (0x0023)
            ADCCMP2L  = 0x04,    // Base + 0x04 (0x0024)
            ADCM1CFG  = 0x05,    // Base + 0x05 (0x0025)
            ADCM2CFG  = 0x06,
            ADCM3CFG  = 0x07,
            ADCM4CFG  = 0x08,
            ADCM5CFG  = 0x09,
            ADCM6CFG  = 0x0A,
            ADCM7CFG  = 0x0B,
            ADCM8CFG  = 0x0C,
            ADCM9CFG  = 0x0D,
            ADCM10CFG = 0x0E,
            ADCM11CFG = 0x0F,
            ADCM12CFG = 0x10,    // Base + 0x10 (0x0030)
            ADCBUF1   = 0x11,    // Base + 0x11 (0x0031)
            ADCBUF2   = 0x12,
            ADCBUF3   = 0x13,
            ADCBUF4   = 0x14,
            ADCBUF5   = 0x15,
            ADCBUF6   = 0x16,
            ADCBUF7   = 0x17,
            ADCBUF8   = 0x18,
            ADCBUF9   = 0x19,
            ADCBUF10  = 0x1A,
            ADCBUF11  = 0x1B,
            ADCBUF12  = 0x1C,    // Base + 0x1C (0x003C)
        };

        enum I2Cx : uint8_t
        {
            I2CCTRL  = 0x00,
            I2CTRANS = 0x01,
            I2CTX    = 0x04,    // 16 registers
            I2CRX    = 0x14,    // 16 registers
        };

        enum TIMx : uint8_t
        {
            TIMxCR    = 0x00,
            TIMxCCMR1 = 0x01,
            TIMxCCMR2 = 0x02,
            TIMxCCMR3 = 0x03,
            TIMxCCMR4 = 0x04,
            TIMxIER   = 0x05,
            TIMxPSC   = 0x06,
            TIMxCNT   = 0x07,
            TIMxARR   = 0x08,
            TIMxCCR1  = 0x09,
            TIMxCCR2  = 0x0A,
            TIMxCCR3  = 0x0B,
            TIMxCCR4  = 0x0C,
        };
    };
}    // namespace AsnPlus

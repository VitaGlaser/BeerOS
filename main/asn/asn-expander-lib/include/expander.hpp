#pragma once

#include "adc/adc.hpp"
#include "asn/asn-core/logger.hpp"
#include "gateway/i2c_gateway.hpp"
#include "gpio/port.hpp"
#include "register_addresses.hpp"
#include "system/system.hpp"
#include "timer/timer.hpp"
#include "transport.hpp"
#include "u16_register.hpp"
#include <cstdint>

namespace AsnPlus::Expander
{
    class Expander
    {
    public:
        Expander( Transport & transport );

        void initialize();
        void poll();

        Gpio::Port &       getPortA();
        Gpio::Port &       getPortB();
        Adc::Adc &         getAdc();
        SystemPeripheral & getSystem();
        I2CGateway &       getI2CGateway();
        Timers::Timer &    getTimerA();
        Timers::Timer &    getTimerB();
        Timers::Timer &    getTimerC();
        Timers::Timer &    getTimerD();
        Timers::Timer &    getTimerE();

    private:
        static constexpr const char TAG[] = "Expander::Expander";
        using Log                         = Logger< 0, TAG >;

        Transport &     _transport;
        RegisterContext _context { _transport };

        Gpio::Port _portA { _context, RegisterAddresses::Peripherals::PER_IOA };
        Gpio::Port _portB { _context, RegisterAddresses::Peripherals::PER_IOB };

        Adc::Adc _adc { _context, RegisterAddresses::Peripherals::PER_ADC };

        SystemPeripheral _system { _context, RegisterAddresses::Peripherals::PER_SYSTEM };

        I2CGateway _i2cGateway { _context, RegisterAddresses::Peripherals::PER_I2C };

        // TODO: Timers A-C, E

        Timers::Timer _timerA { _context, RegisterAddresses::Peripherals::PER_TIMERA };
        Timers::Timer _timerB { _context, RegisterAddresses::Peripherals::PER_TIMERB };
        Timers::Timer _timerC { _context, RegisterAddresses::Peripherals::PER_TIMERC };
        Timers::Timer _timerD { _context, RegisterAddresses::Peripherals::PER_TIMERD };
        Timers::Timer _timerE { _context, RegisterAddresses::Peripherals::PER_TIMERE };

        // TODO:
        //  ISR
        //  ADC
        //  PWM Channels

        void _loadAll();
    };
}    // namespace AsnPlus::Expander

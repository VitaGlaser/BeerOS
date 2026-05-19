#pragma once

#include "adc/adc.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-hal/include/peripherals/gpio.hpp"
#include "bootloader/bootloader.hpp"
#include "bootloader/flasher.hpp"
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
        enum class UpdateStrategy
        {
            BootPin,
            Software,
            None
        };

        Expander( Transport & transport, UpdateStrategy updateStrategy, IGpio * nrst = nullptr, IGpio * boot = nullptr );

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
        IGpio *         _nrst;
        IGpio *         _boot;
        UpdateStrategy  _updateStrategy;
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

        Stm32Bootloader::GenericFlasher _flasher {
            _transport,
            DeviceDefinitions::STM32C051_64K::FlashSize,
            DeviceDefinitions::STM32C051_64K::FlashAddress
        };
        // TODO:
        //  ISR
        //  ADC
        //  PWM Channels

        void _loadAll();
        void _resetNormal();
        void _resetToBootloader();
    };
}    // namespace AsnPlus::Expander

#pragma once

#include "asn/asn-hal/include/peripherals/gpio.hpp"

#include "../gpio/port.hpp"

namespace AsnPlus::Expander
{
    class PortPinGpio final : public IGpio
    {
    public:
        struct Config : public IGpio::Config
        {
            bool pullupEnabled   = false;
            bool pulldownEnabled = false;
        };

        PortPinGpio( Config & config, Gpio::Port::Pin pin );

        bool initialize() override;
        void poll() override;
        void set( bool value ) override;
        bool get() override;
        void toggle() override;
        bool enableInterrupt() override;
        bool disableInterrupt() override;

    private:
        Config &        _config;
        Gpio::Port::Pin _pin;
        bool            _state = false;

        bool _readInput();
    };
}    // namespace AsnPlus::Expander

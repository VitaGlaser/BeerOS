#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "../peripherals/gpio.hpp"

namespace AsnPlus
{
    class RotaryEncoder
    {
    public:
        using TurnDelegate  = Delegate< void() >;
        using PressDelegate = Delegate< void() >;

        struct Config
        {
            bool   invertDirection = false;
            int8_t stepsPerDetent  = 4;
        };

        static constexpr IGpio::Config GPIO_A_CONFIG {
            .mode          = IGpio::Config::PinMode::INPUT,
            .interruptType = IGpio::Config::InterruptType::CHANGE,
        };

        static constexpr IGpio::Config GPIO_B_CONFIG {
            .mode          = IGpio::Config::PinMode::INPUT,
            .interruptType = IGpio::Config::InterruptType::CHANGE,
        };

        RotaryEncoder( Config & config, IGpio & gpioA, IGpio & gpioB, IGpio * button = nullptr );

        bool initialize();

        void setClockwiseDelegate( const TurnDelegate & cb ) { _onClockwise = cb; }

        void setCounterClockwiseDelegate( const TurnDelegate & cb ) { _onCounterClockwise = cb; }

        void setPressDelegate( const PressDelegate & cb ) { _onPress = cb; }

        void poll() {}

    private:
        static constexpr const char TAG[] = "RotaryEncoder";
        using Log                         = Logger< ModuleConfig::Hal::LOG_LEVEL, TAG >;

        Config & _config;

        IGpio & _gpioA;
        IGpio & _gpioB;
        IGpio * _button;

        int8_t  _stepAccum   = 0;
        uint8_t _lastABState = 0;

        TurnDelegate  _onClockwise;
        TurnDelegate  _onCounterClockwise;
        PressDelegate _onPress;

        uint8_t _readAB() const;
        void    _handleABChange();

        void _onAChange( bool level );
        void _onBChange( bool level );
        void _onButtonPress( bool level );

        void _emitClockwise();
        void _emitCounterClockwise();
        void _emitPress();
    };
}    // namespace AsnPlus

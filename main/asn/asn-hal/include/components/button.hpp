#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/utils.hpp"

#include "peripherals/gpio.hpp"

namespace AsnPlus
{
    class Button
    {
    public:
        using ClickDelegate      = Delegate< void() >;
        using LongPressDelegate  = Delegate< void() >;
        using MultiClickDelegate = Delegate< void( uint8_t count ) >;

        struct Config
        {
            uint32_t debounceMs      = 25;
            uint32_t longPressMs     = 1000;
            uint32_t multiClickGapMs = 100;

            uint8_t maxMultiClicks   = 5;

            // false = active-low (pull-up, pressed = 0)
            // true  = active-high (pressed = 1)
            bool inverted            = false;
        };

        static constexpr IGpio::Config GPIO_CONFIG {
            .mode          = IGpio::Config::PinMode::INPUT,
            .interruptType = IGpio::Config::InterruptType::CHANGE,
        };

        Button( Config & config, IGpio & gpio );

        bool initialize();
        void poll();

        void setClickDelegate( const ClickDelegate & cb ) { _onClick = cb; }

        void setLongPressDelegate( const LongPressDelegate & cb ) { _onLong = cb; }

        void setMultiClickDelegate( const MultiClickDelegate & cb ) { _onMultiClick = cb; }

        bool isPressed() const { return _stableDown; }

    private:
        static constexpr const char TAG[] = "Button";
        using Log                         = Logger< ModuleConfig::Hal::LOG_LEVEL, TAG >;

        Config & _config;
        IGpio &  _gpio;

        volatile bool     _rawLevel      = false;
        volatile uint32_t _lastRawChange = 0;

        bool _stableDown                 = false;

        uint32_t _pressStart             = 0;
        uint32_t _lastUpTime             = 0;
        uint8_t  _clickCount             = 0;
        bool     _pendingClick           = false;
        bool     _longFired              = false;

        ClickDelegate      _onClick;
        LongPressDelegate  _onLong;
        MultiClickDelegate _onMultiClick;

        bool _isPressedLevel( bool level ) const { return _config.inverted ? level : ! level; }

        void _onGpioChange( bool level );
        void _processDebounce( uint32_t now );
        void _processLongPress( uint32_t now );
        void _processFinalizeClicks( uint32_t now );
    };
}    // namespace AsnPlus

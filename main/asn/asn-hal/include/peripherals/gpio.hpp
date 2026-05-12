#pragma once

#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/types.hpp"

namespace AsnPlus
{
    class IGpio
    {
    public:
        struct Config
        {
            enum class PinMode
            {
                INPUT,
                OUTPUT
            };

            enum class InterruptType
            {
                NONE,
                RISING,
                FALLING,
                CHANGE
            };

            PinMode       mode;
            InterruptType interruptType = InterruptType::NONE;
        };

        using InterruptDelegate = Delegate< void( bool level ) >;

        IGpio( Config & config ) : _config( config ) {}

        virtual bool initialize() = 0;

        virtual void poll() {}

        virtual void set( bool value )  = 0;
        virtual bool get()              = 0;
        virtual void toggle()           = 0;

        virtual bool enableInterrupt()  = 0;
        virtual bool disableInterrupt() = 0;

        void setInterruptDelegate( const InterruptDelegate & cb ) { _interruptDelegate = cb; }

    protected:
        Config &          _config;
        InterruptDelegate _interruptDelegate;

        void _emitInterrupt( bool level )
        {
            if ( _interruptDelegate.is_valid() )
            {
                _interruptDelegate( level );
            }
        }
    };
}    // namespace AsnPlus

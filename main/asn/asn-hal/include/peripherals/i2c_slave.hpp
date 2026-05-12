#pragma once

#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/types.hpp"

namespace AsnPlus
{
    class II2cSlave
    {
    public:
        struct Config
        {
            uint8_t address;
        };

        // Delegates instead of raw function pointers
        using OnReceiveDelegate = Delegate< void( const uint8_t * data, size_t len ) >;
        using OnRequestDelegate = Delegate< void() >;

        II2cSlave( Config & config ) : _config( config ) {}

        virtual bool initialize()                                                   = 0;

        virtual int32_t write( const uint8_t * data, size_t len, uint32_t timeout ) = 0;
        virtual int32_t read( uint8_t * data, size_t len, uint32_t timeout )        = 0;

        // Setters for delegates
        void setOnReceiveCallback( OnReceiveDelegate callback ) { _onReceiveCallback = callback; }

        void setOnRequestCallback( OnRequestDelegate callback ) { _onRequestCallback = callback; }

    protected:
        static constexpr const char TAG[] = "II2cSlave";
        Config &                    _config;
        OnReceiveDelegate           _onReceiveCallback {};
        OnRequestDelegate           _onRequestCallback {};

    private:
    };
}    // namespace AsnPlus

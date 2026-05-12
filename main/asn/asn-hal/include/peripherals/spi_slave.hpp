#pragma once

#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/types.hpp"

namespace AsnPlus
{
    class ISpiSlave
    {
    public:
        struct Config
        {
        };

        // Delegates instead of raw function pointers
        using PostSetupDelegate       = Delegate< void( void * context ) >;
        using PostTransactionDelegate = Delegate< void( void * context ) >;

        ISpiSlave( Config & config ) : _config( config ) {}

        virtual bool initialize()                                                   = 0;

        virtual int32_t write( const uint8_t * data, size_t len, uint32_t timeout ) = 0;
        virtual int32_t read( uint8_t * data, size_t len, uint32_t timeout )        = 0;

        void setPostSetupDelegate( PostSetupDelegate callback ) { _postSetupDelegate = callback; }

        void setPostTransactionDelegate( PostTransactionDelegate callback ) { _postTransactionDelegate = callback; }

    protected:
        static constexpr const char TAG[] = "ISpiSlave";
        Config &                    _config;

        PostSetupDelegate       _postSetupDelegate {};
        PostTransactionDelegate _postTransactionDelegate {};

    private:
    };
}    // namespace AsnPlus

#pragma once

#include "asn/asn-core/types.hpp"
#include "asn/asn-core/vector.hpp"

#include "structs.hpp"

namespace AsnPlus::Https
{
    class IClient
    {
    public:
        virtual bool initialize()                                            = 0;
        
        virtual bool setHeaders( const IVector< HeaderKeyValue > & headers ) = 0;

        virtual void cleanup()                                               = 0;

        uint16_t request( const char * url, Request * request, Response * response );

    protected:
        virtual uint16_t get( const char * url, Response * response )                     = 0;
        virtual uint16_t post( const char * url, Request * request, Response * response ) = 0;

    private:
        static constexpr const char TAG[] = "IClient";
    };
}    // namespace AsnPlus::Https

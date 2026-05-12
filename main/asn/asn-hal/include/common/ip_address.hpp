#pragma once

#include "asn/asn-core/string.hpp"
#include "asn/asn-core/types.hpp"

namespace AsnPlus
{
    struct IpAddress
    {
        static constexpr size_t STRING_LENGTH = 16;    // Max length for "xxx.xxx.xxx.xxx\0"

        uint8_t octets[ 4 ];

        constexpr IpAddress( uint8_t a, uint8_t b, uint8_t c, uint8_t d ) : octets { a, b, c, d } {}

        constexpr IpAddress() : octets { 0, 0, 0, 0 } {}

        constexpr IpAddress( uint32_t ip ) :
            octets {
                static_cast< uint8_t >( ip & 0xFF ),
                static_cast< uint8_t >( ( ip >> 8 ) & 0xFF ),
                static_cast< uint8_t >( ( ip >> 16 ) & 0xFF ),
                static_cast< uint8_t >( ( ip >> 24 ) & 0xFF )
            }
        {
        }

        void toString( char * buffer, size_t size ) const;
        String< STRING_LENGTH > toString() const;

        uint32_t toNetworkOrder() const;
        uint32_t toUint32() const;
        void     toArrayInNetworkOrder( uint8_t * arr );

        static IpAddress fromString( const char * ipStr );

        bool operator==( const IpAddress & other ) const
        {
            return octets[ 0 ] == other.octets[ 0 ] && octets[ 1 ] == other.octets[ 1 ] &&
                octets[ 2 ] == other.octets[ 2 ] && octets[ 3 ] == other.octets[ 3 ];
        }

        bool operator!=( const IpAddress & other ) const { return ! ( *this == other ); }

        bool isValid() const;
        bool isPrivate() const;

        static constexpr IpAddress empty() { return IpAddress( 0, 0, 0, 0 ); }
    };
}    // namespace AsnPlus

#pragma once

#include "asn/asn-core/types.hpp"
#include "host/ble_uuid.h"

namespace AsnPlus
{
    static constexpr u8 byte( const char * digits )
    {
        return ( digits[ 0 ] <= '9' ? digits[ 0 ] - '0' : digits[ 0 ] - 'a' + 10 ) * 16 +
            ( digits[ 1 ] <= '9' ? digits[ 1 ] - '0' : digits[ 1 ] - 'a' + 10 );
    }

    static constexpr ble_uuid128_t operator""_uuid( const char * text, size_t size )
    {
        if ( size == (size_t) 4 )
        {
            return {
                { BLE_UUID_TYPE_128 },
                {
                 0xfb, 0x34,
                 0x9b, 0x5f,
                 0x80, 0x00,
                 0x00, 0x80,
                 0x00, 0x10,
                 0x00, 0x00,
                 byte( text + 2 ),
                 byte( text + 0 ),
                 0x00, 0x00,
                 }
            };
        }
        return {
            { BLE_UUID_TYPE_128 },
            {
             byte( text + 34 ),
             byte( text + 32 ),
             byte( text + 30 ),
             byte( text + 28 ),
             byte( text + 26 ),
             byte( text + 24 ),
             byte( text + 21 ),
             byte( text + 19 ),
             byte( text + 16 ),
             byte( text + 14 ),
             byte( text + 11 ),
             byte( text + 9 ),
             byte( text + 6 ),
             byte( text + 4 ),
             byte( text + 2 ),
             byte( text + 0 ),
             }
        };
    }
}    // namespace AsnPlus

#pragma once

#include "asn/asn-core/string.hpp"
#include "asn/asn-core/types.hpp"

namespace AsnPlus::Wifi
{
    enum class AuthMode : uint8_t
    {
        OPEN,
        WEP,
        WPA,
        WPA2,
        WPA_WPA2
    };

    enum class Channel : uint8_t
    {
        CHANNEL_2_4,
        CHANNEL_5
    };

    struct WifiConfig
    {
        static constexpr uint8_t MAC_ADDRESS_LENGTH  = 6;
        static constexpr uint8_t MAX_SSID_LENGTH     = 32;
        static constexpr uint8_t MAX_PASSWORD_LENGTH = 64;

        uint8_t                       macAddress[ MAC_ADDRESS_LENGTH ];
        String< MAX_SSID_LENGTH >     ssid;
        String< MAX_PASSWORD_LENGTH > password;

        int      rssi     = 0;
        AuthMode authMode = AuthMode::OPEN;
        Channel  channel  = Channel::CHANNEL_2_4;

        void serialize( uint8_t * data ) const
        {
            memcpy( data, macAddress, MAC_ADDRESS_LENGTH );
            data += MAC_ADDRESS_LENGTH;

            // SSID slot (zero-padded)
            memset( data, 0, MAX_SSID_LENGTH );
            memcpy( data, ssid.data(), ssid.size() );
            data += MAX_SSID_LENGTH;

            // Password slot (zero-padded)
            memset( data, 0, MAX_PASSWORD_LENGTH );
            memcpy( data, password.data(), password.size() );
            data += MAX_PASSWORD_LENGTH;

            // RSSI (2 bytes)
            memcpy( data, &rssi, sizeof( rssi ) );
            data   += sizeof( rssi );

            *data++ = static_cast< uint8_t >( authMode );
            *data++ = static_cast< uint8_t >( channel );
        }

        void deserialize( const uint8_t * data )
        {
            memcpy( macAddress, data, MAC_ADDRESS_LENGTH );
            data += MAC_ADDRESS_LENGTH;

            ssid.assign( reinterpret_cast< const char * >( data ), MAX_SSID_LENGTH );
            data += MAX_SSID_LENGTH;

            password.assign( reinterpret_cast< const char * >( data ), MAX_PASSWORD_LENGTH );
            data    += MAX_PASSWORD_LENGTH;

            rssi     = *data++;
            authMode = static_cast< AuthMode >( *data++ );
            channel  = static_cast< Channel >( *data++ );
        }
    };
}    // namespace AsnPlus::Wifi

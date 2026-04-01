#pragma once

#include "cJSON.h"

#include "asn/asn-core/types.hpp"

#include "asn/asn-hal/common/common_structs.hpp"

#include "components/networking/ethernet.hpp"

#include "components/cloud/json_conversion/device_config.hpp"

namespace AsnPlus::Cloud
{
    namespace EthernetConfigJson
    {
        static constexpr const char USE_DHCP_TAG[] = "useDhcp";
        static constexpr const char IP_TAG[]       = "ip";
        static constexpr const char GATEWAY_TAG[]  = "gateway";
        static constexpr const char NETMASK_TAG[]  = "netmask";
    }    // namespace EthernetConfigJson

    namespace WifiConfigJson
    {
        static constexpr const char SSID_TAG[]     = "ssid";
        static constexpr const char PASSWORD_TAG[] = "password";
    }    // namespace WifiConfigJson

    namespace LteConfigJson
    {
        static constexpr const char ENABLED_TAG[] = "enabled";
    }    // namespace LteConfigJson

    struct LteConfig : AsnPlus::Config
    {
        bool enabled = false;
    };

    struct WifiNetworkConfig
    {
        static constexpr size_t SSID_LENGTH     = 32;
        static constexpr size_t PASSWORD_LENGTH = 64;
        char                    ssid[ SSID_LENGTH ] {};
        char                    password[ PASSWORD_LENGTH ] {};
    };

    struct ConnectionConfig
    {
        uint64_t                          timestamp = 0;
        Networking::W5500Ethernet::Config ethernetConfig {};
        WifiNetworkConfig                 wifiConfig {};
        LteConfig                         lteConfig {};
    };

    void toJson( Networking::W5500Ethernet::Config & config, cJSON * json )
    {
        cJSON_AddBoolToObject( json, EthernetConfigJson::USE_DHCP_TAG, config.useDHCP );

        char ip_str[ 16 ];
        config.ip.toString( ip_str, sizeof( ip_str ) );
        cJSON_AddStringToObject( json, EthernetConfigJson::IP_TAG, ip_str );

        char gateway_str[ 16 ];
        config.gateway.toString( gateway_str, sizeof( gateway_str ) );
        cJSON_AddStringToObject( json, EthernetConfigJson::GATEWAY_TAG, gateway_str );

        char mask_str[ 16 ];
        config.mask.toString( mask_str, sizeof( mask_str ) );
        cJSON_AddStringToObject( json, EthernetConfigJson::NETMASK_TAG, mask_str );
    }

    void fromJson( Networking::W5500Ethernet::Config & config, cJSON * json )
    {
        config.useDHCP = cJSON_IsTrue( cJSON_GetObjectItem( json, EthernetConfigJson::USE_DHCP_TAG ) );

        auto parseIp   = []( cJSON * item ) -> Networking::IPAddress
        {
            if ( ! item || ! cJSON_IsString( item ) ) return {};
            unsigned int a = 0, b = 0, c = 0, d = 0;
            sscanf( item->valuestring, "%u.%u.%u.%u", &a, &b, &c, &d );
            return Networking::IPAddress(
                static_cast< uint8_t >( a ),
                static_cast< uint8_t >( b ),
                static_cast< uint8_t >( c ),
                static_cast< uint8_t >( d )
            );
        };

        config.ip      = parseIp( cJSON_GetObjectItem( json, EthernetConfigJson::IP_TAG ) );
        config.gateway = parseIp( cJSON_GetObjectItem( json, EthernetConfigJson::GATEWAY_TAG ) );
        config.mask    = parseIp( cJSON_GetObjectItem( json, EthernetConfigJson::NETMASK_TAG ) );
    }

    void toJson( WifiNetworkConfig & config, cJSON * json )
    {
        cJSON_AddStringToObject( json, WifiConfigJson::SSID_TAG, config.ssid );
        cJSON_AddStringToObject( json, WifiConfigJson::PASSWORD_TAG, config.password );
    }

    void fromJson( WifiNetworkConfig & config, cJSON * json )
    {
        cJSON * ssid = cJSON_GetObjectItem( json, WifiConfigJson::SSID_TAG );
        if ( ssid && cJSON_IsString( ssid ) )
            strncpy( config.ssid, ssid->valuestring, WifiNetworkConfig::SSID_LENGTH - 1 );

        cJSON * password = cJSON_GetObjectItem( json, WifiConfigJson::PASSWORD_TAG );
        if ( password && cJSON_IsString( password ) )
            strncpy( config.password, password->valuestring, WifiNetworkConfig::PASSWORD_LENGTH - 1 );
    }

    void toJson( LteConfig & config, cJSON * json )
    {
        cJSON_AddBoolToObject( json, LteConfigJson::ENABLED_TAG, config.enabled );
    }

    void fromJson( LteConfig & config, cJSON * json )
    {
        config.enabled = cJSON_IsTrue( cJSON_GetObjectItem( json, LteConfigJson::ENABLED_TAG ) );
    }

    void toJson( ConnectionConfig & config, cJSON * json )
    {
        cJSON_AddNumberToObject( json, DeviceConfigJson::TIMESTAMP_TAG, static_cast< double >( config.timestamp ) );

        cJSON * ethernetConfigJson = cJSON_CreateObject();
        toJson( config.ethernetConfig, ethernetConfigJson );
        cJSON_AddItemToObject( json, "ethernetConfig", ethernetConfigJson );

        cJSON * wifiConfigJson = cJSON_CreateObject();
        toJson( config.wifiConfig, wifiConfigJson );
        cJSON_AddItemToObject( json, "wifiConfig", wifiConfigJson );

        cJSON * lteConfigJson = cJSON_CreateObject();
        toJson( config.lteConfig, lteConfigJson );
        cJSON_AddItemToObject( json, "lteConfig", lteConfigJson );
    }

    void fromJson( ConnectionConfig & config, cJSON * json )
    {
        config.timestamp = static_cast< uint64_t >(
            cJSON_GetNumberValue( cJSON_GetObjectItem( json, DeviceConfigJson::TIMESTAMP_TAG ) )
        );

        cJSON * ethernetConfigJson = cJSON_GetObjectItem( json, "ethernetConfig" );
        if ( ethernetConfigJson && cJSON_IsObject( ethernetConfigJson ) )
            fromJson( config.ethernetConfig, ethernetConfigJson );

        cJSON * wifiConfigJson = cJSON_GetObjectItem( json, "wifiConfig" );
        if ( wifiConfigJson && cJSON_IsObject( wifiConfigJson ) ) fromJson( config.wifiConfig, wifiConfigJson );

        cJSON * lteConfigJson = cJSON_GetObjectItem( json, "lteConfig" );
        if ( lteConfigJson && cJSON_IsObject( lteConfigJson ) ) fromJson( config.lteConfig, lteConfigJson );
    }
}    // namespace AsnPlus::Cloud

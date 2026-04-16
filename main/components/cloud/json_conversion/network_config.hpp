#pragma once

#include "cJSON.h"

#include "asn/asn-core/types.hpp"

#include "asn/asn-hal/common/common_structs.hpp"

#include "asn/asn-esp32-wifi/ethernet/ethernet.hpp"

#include "components/cloud/json_conversion/device_config.hpp"

namespace AsnPlus::Cloud
{
    // MARK: JSON tags

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

    namespace RequestConfigJson
    {
        static constexpr const char TIMESTAMP_TAG[]                 = "timestamp";
        static constexpr const char ETH_EVENT_RETRY_INTERVAL_TAG[]  = "ethEventRetryInterval";
        static constexpr const char ETH_STATE_INTERVAL_TAG[]        = "ethStatusInterval";
        static constexpr const char WIFI_EVENT_RETRY_INTERVAL_TAG[] = "wifiEventRetryInterval";
        static constexpr const char WIFI_STATE_INTERVAL_TAG[]       = "wifiStatusInterval";
        static constexpr const char LTE_EVENT_RETRY_INTERVAL_TAG[]  = "lteEventRetryInterval";
        static constexpr const char LTE_STATE_INTERVAL_TAG[]        = "lteStatusInterval";
    }    // namespace RequestConfigJson

    namespace NetworkConfigJson
    {
        static constexpr const char TIMESTAMP_TAG[]       = "timestamp";
        static constexpr const char ETHERNET_CONFIG_TAG[] = "ethernetConfig";
        static constexpr const char WIFI_CONFIG_TAG[]     = "wifiConfig";
        static constexpr const char LTE_CONFIG_TAG[]      = "lteConfig";
        static constexpr const char REQUEST_CONFIG_TAG[]  = "requestConfig";
    }    // namespace NetworkConfigJson

    // MARK: Structs

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

    struct RequestConfig
    {
        uint64_t timestamp              = 0;
        uint32_t ethEventRetryInterval  = 0;
        uint32_t ethStatusInterval      = 0;
        uint32_t wifiEventRetryInterval = 0;
        uint32_t wifiStatusInterval     = 0;
        uint32_t lteEventRetryInterval  = 0;
        uint32_t lteStatusInterval      = 0;
    };

    struct NetworkConfig
    {
        uint64_t                       timestamp = 0;
        Network::W5500Ethernet::Config ethernetConfig {};
        WifiNetworkConfig              wifiConfig {};
        LteConfig                      lteConfig {};
        RequestConfig                  requestConfig {};
    };

    // MARK: toJson / fromJson — EthernetConfig

    void toJson( Network::W5500Ethernet::Config & config, cJSON * json )
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

    void fromJson( Network::W5500Ethernet::Config & config, cJSON * json )
    {
        config.useDHCP = cJSON_IsTrue( cJSON_GetObjectItem( json, EthernetConfigJson::USE_DHCP_TAG ) );

        auto parseIp   = []( cJSON * item ) -> IpAddress
        {
            if ( ! item || ! cJSON_IsString( item ) ) return {};
            unsigned int a = 0, b = 0, c = 0, d = 0;
            sscanf( item->valuestring, "%u.%u.%u.%u", &a, &b, &c, &d );
            return IpAddress(
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

    // MARK: toJson / fromJson — WifiConfig

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

    // MARK: toJson / fromJson — LteConfig

    void toJson( LteConfig & config, cJSON * json )
    {
        cJSON_AddBoolToObject( json, LteConfigJson::ENABLED_TAG, config.enabled );
    }

    void fromJson( LteConfig & config, cJSON * json )
    {
        config.enabled = cJSON_IsTrue( cJSON_GetObjectItem( json, LteConfigJson::ENABLED_TAG ) );
    }

    // MARK: toJson / fromJson — RequestConfig

    void toJson( RequestConfig & config, cJSON * json )
    {
        cJSON_AddNumberToObject( json, RequestConfigJson::TIMESTAMP_TAG, static_cast< double >( config.timestamp ) );
        cJSON_AddNumberToObject( json, RequestConfigJson::ETH_EVENT_RETRY_INTERVAL_TAG, config.ethEventRetryInterval );
        cJSON_AddNumberToObject( json, RequestConfigJson::ETH_STATE_INTERVAL_TAG, config.ethStatusInterval );
        cJSON_AddNumberToObject(
            json, RequestConfigJson::WIFI_EVENT_RETRY_INTERVAL_TAG, config.wifiEventRetryInterval
        );
        cJSON_AddNumberToObject( json, RequestConfigJson::WIFI_STATE_INTERVAL_TAG, config.wifiStatusInterval );
        cJSON_AddNumberToObject( json, RequestConfigJson::LTE_EVENT_RETRY_INTERVAL_TAG, config.lteEventRetryInterval );
        cJSON_AddNumberToObject( json, RequestConfigJson::LTE_STATE_INTERVAL_TAG, config.lteStatusInterval );
    }

    void fromJson( RequestConfig & config, cJSON * json )
    {
        config.timestamp = static_cast< uint64_t >(
            cJSON_GetNumberValue( cJSON_GetObjectItem( json, RequestConfigJson::TIMESTAMP_TAG ) )
        );

        cJSON * ethEventRetry = cJSON_GetObjectItem( json, RequestConfigJson::ETH_EVENT_RETRY_INTERVAL_TAG );
        if ( ethEventRetry && cJSON_IsNumber( ethEventRetry ) )
            config.ethEventRetryInterval = static_cast< uint32_t >( ethEventRetry->valuedouble );

        cJSON * ethStatus = cJSON_GetObjectItem( json, RequestConfigJson::ETH_STATE_INTERVAL_TAG );
        if ( ethStatus && cJSON_IsNumber( ethStatus ) )
            config.ethStatusInterval = static_cast< uint32_t >( ethStatus->valuedouble );

        cJSON * wifiEventRetry = cJSON_GetObjectItem( json, RequestConfigJson::WIFI_EVENT_RETRY_INTERVAL_TAG );
        if ( wifiEventRetry && cJSON_IsNumber( wifiEventRetry ) )
            config.wifiEventRetryInterval = static_cast< uint32_t >( wifiEventRetry->valuedouble );

        cJSON * wifiStatus = cJSON_GetObjectItem( json, RequestConfigJson::WIFI_STATE_INTERVAL_TAG );
        if ( wifiStatus && cJSON_IsNumber( wifiStatus ) )
            config.wifiStatusInterval = static_cast< uint32_t >( wifiStatus->valuedouble );

        cJSON * lteEventRetry = cJSON_GetObjectItem( json, RequestConfigJson::LTE_EVENT_RETRY_INTERVAL_TAG );
        if ( lteEventRetry && cJSON_IsNumber( lteEventRetry ) )
            config.lteEventRetryInterval = static_cast< uint32_t >( lteEventRetry->valuedouble );

        cJSON * lteStatus = cJSON_GetObjectItem( json, RequestConfigJson::LTE_STATE_INTERVAL_TAG );
        if ( lteStatus && cJSON_IsNumber( lteStatus ) )
            config.lteStatusInterval = static_cast< uint32_t >( lteStatus->valuedouble );
    }

    // MARK: toJson / fromJson — NetworkConfig

    void toJson( NetworkConfig & config, cJSON * json )
    {
        cJSON_AddNumberToObject( json, NetworkConfigJson::TIMESTAMP_TAG, static_cast< double >( config.timestamp ) );

        cJSON * ethernetConfigJson = cJSON_CreateObject();
        toJson( config.ethernetConfig, ethernetConfigJson );
        cJSON_AddItemToObject( json, NetworkConfigJson::ETHERNET_CONFIG_TAG, ethernetConfigJson );

        cJSON * wifiConfigJson = cJSON_CreateObject();
        toJson( config.wifiConfig, wifiConfigJson );
        cJSON_AddItemToObject( json, NetworkConfigJson::WIFI_CONFIG_TAG, wifiConfigJson );

        cJSON * lteConfigJson = cJSON_CreateObject();
        toJson( config.lteConfig, lteConfigJson );
        cJSON_AddItemToObject( json, NetworkConfigJson::LTE_CONFIG_TAG, lteConfigJson );

        cJSON * requestConfigJson = cJSON_CreateObject();
        toJson( config.requestConfig, requestConfigJson );
        cJSON_AddItemToObject( json, NetworkConfigJson::REQUEST_CONFIG_TAG, requestConfigJson );
    }

    void fromJson( NetworkConfig & config, cJSON * json )
    {
        config.timestamp = static_cast< uint64_t >(
            cJSON_GetNumberValue( cJSON_GetObjectItem( json, NetworkConfigJson::TIMESTAMP_TAG ) )
        );

        cJSON * ethernetConfigJson = cJSON_GetObjectItem( json, NetworkConfigJson::ETHERNET_CONFIG_TAG );
        if ( ethernetConfigJson && cJSON_IsObject( ethernetConfigJson ) )
            fromJson( config.ethernetConfig, ethernetConfigJson );

        cJSON * wifiConfigJson = cJSON_GetObjectItem( json, NetworkConfigJson::WIFI_CONFIG_TAG );
        if ( wifiConfigJson && cJSON_IsObject( wifiConfigJson ) ) fromJson( config.wifiConfig, wifiConfigJson );

        cJSON * lteConfigJson = cJSON_GetObjectItem( json, NetworkConfigJson::LTE_CONFIG_TAG );
        if ( lteConfigJson && cJSON_IsObject( lteConfigJson ) ) fromJson( config.lteConfig, lteConfigJson );

        cJSON * requestConfigJson = cJSON_GetObjectItem( json, NetworkConfigJson::REQUEST_CONFIG_TAG );
        if ( requestConfigJson && cJSON_IsObject( requestConfigJson ) )
            fromJson( config.requestConfig, requestConfigJson );
    }
}    // namespace AsnPlus::Cloud

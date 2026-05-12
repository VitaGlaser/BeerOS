#pragma once

// W5500 Ethernet network layer — IP config, DHCP/static, TCP/IP stack attachment.

#include "asn_module_config.hpp"

#include "esp_eth_netif_glue.h"
#include "esp_eth_spec.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_types.h"

#include "asn/asn-core/logger.hpp"

#include "asn/asn-hal/include/common/common_structs.hpp"
#include "asn/asn-hal/include/common/ip_address.hpp"

#include "w5500_sta.hpp"

namespace AsnPlus::Network
{
    class W5500Ethernet
    {
    public:
        enum class State : uint8_t
        {
            UNKNOWN,
            SCANNING,
            SCAN_DONE,
            CONNECTING,
            CONNECTED,
            DISCONNECTING,
            DISCONNECTED,
            ERROR
        };

        struct Config : AsnPlus::Config
        {
            bool      useDHCP = true;
            IpAddress ip { 0, 0, 0, 0 };
            IpAddress mask { 0, 0, 0, 0 };
            IpAddress gateway { 0, 0, 0, 0 };
        };

        struct Runtime : AsnPlus::Runtime
        {
            State state = State::UNKNOWN;
            IpAddress ip { 0, 0, 0, 0 };
            IpAddress mask { 0, 0, 0, 0 };
            IpAddress gateway { 0, 0, 0, 0 };
        };

        // Legacy unified status view for connection manager compatibility
        struct Status
        {
            uint8_t   mac[ ETH_ADDR_LEN ];
            bool      linkUp;
            IpAddress ip { 0, 0, 0, 0 };
            IpAddress mask { 0, 0, 0, 0 };
            IpAddress gateway { 0, 0, 0, 0 };
        };

        W5500Ethernet( Config & config, Runtime & runtime, W5500Sta & sta ) :
            _config( config ),
            _runtime( runtime ),
            _sta( sta )
        {
        }

        void          initialize();
        void          start();
        esp_netif_t * getNetif();
        Status        getStatus();
        Config        getConfig();
        void          setConfig( Config config );

    private:
        static constexpr const char TAG[] = "W5500Ethernet";
        using Log                         = AsnPlus::Logger< ModuleConfig::Network::LOG_LEVEL, TAG >;

        Config &   _config;
        Runtime &  _runtime;
        W5500Sta & _sta;

        esp_netif_t * _netif = nullptr;

        static void _ethIpEventCb( void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data )
        {
            static_cast< W5500Ethernet * >( arg )->_ethIpEventHandler( event_id, event_data );
        }

        void _ethIpEventHandler( int32_t event_id, void * event_data );
        bool _attachTcpIpStack();
        void _setDHCP();
        void _setStatic();
    };

}    // namespace AsnPlus::Network

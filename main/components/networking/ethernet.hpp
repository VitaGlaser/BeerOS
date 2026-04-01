#pragma once

#include "asn/asn-core/logger.hpp"
#include "components/networking/ip_address.hpp"
#include "driver/gpio.h"
#include "esp_eth_driver.h"
#include "esp_eth_netif_glue.h"
#include "esp_eth_spec.h"
#include "esp_event.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "hal/spi_types.h"
#include "program/config.hpp"
#include "pthread.h"
#include "soc/gpio_num.h"

namespace AsnPlus::Networking
{
    class Netif
    {
    public:
        static void initialize()
        {
            ESP_ERROR_CHECK( esp_netif_init() );
            ESP_ERROR_CHECK( esp_event_loop_create_default() );
        }
    };

    class W5500Ethernet
    {
    public:
        struct Config
        {
            bool      useDHCP = true;
            IPAddress ip { 0, 0, 0, 0 };
            IPAddress mask { 0, 0, 0, 0 };
            IPAddress gateway { 0, 0, 0, 0 };
        };

        struct Runtime
        {
            uint8_t   mac[ ETH_ADDR_LEN ];
            bool      linkUp;
            IPAddress ip { 0, 0, 0, 0 };
            IPAddress mask { 0, 0, 0, 0 };
            IPAddress gateway { 0, 0, 0, 0 };
        };

        W5500Ethernet(
            spi_host_device_t spiHost,
            gpio_num_t        mosi,
            gpio_num_t        miso,
            gpio_num_t        sck,
            gpio_num_t        cs,
            gpio_num_t        nRst,
            gpio_num_t        intPin,
            int               clockSpeedMHz = 20
        ) :
            _mosi( mosi ),
            _miso( miso ),
            _sck( sck ),
            _cs( cs ),
            _nRst( nRst ),
            _int( intPin ),
            _spiHost( spiHost ),
            _clockSpeed( clockSpeedMHz )
        {
        }

        void initialize()
        {
            // Initialization code for W5500 Ethernet module
            Log::info( "Initializing W5500 Ethernet" );

            Log::debug(
                "Initializing SPI bus(%d) for ethernet at MOSI %d, MISO %d, SCK %d", _spiHost, _mosi, _miso, _sck
            );
            _initSpi();
            Log::info( "Ethernet SPI bus initialized" );

            eth_mac_config_t macCfg                  = ETH_MAC_DEFAULT_CONFIG();
            eth_phy_config_t phyCfg                  = ETH_PHY_DEFAULT_CONFIG();
            phyCfg.phy_addr                          = 0;
            phyCfg.reset_gpio_num                    = _nRst;

            // Configure SPI device
            spi_device_interface_config_t spi_devcfg = {
                .mode = 0, .clock_speed_hz = _clockSpeed * 1000 * 1000, .spics_io_num = _cs, .queue_size = 20
            };

            eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG( _spiHost, &spi_devcfg );
            w5500_config.int_gpio_num       = _int;
            esp_eth_mac_t * mac             = esp_eth_mac_new_w5500( &w5500_config, &macCfg );
            esp_eth_phy_t * phy             = esp_eth_phy_new_w5500( &phyCfg );

            esp_eth_config_t config         = ETH_DEFAULT_CONFIG( mac, phy );    // apply default driver configuration

            Log::debug( "Installing ethernet driver" );
            gpio_install_isr_service( 0 );
            esp_eth_driver_install( &config, &_ethHandle );    // install driver
            Log::info( "Ethernet driver installed succesfully" );

            Log::debug( "Configuring the MAC address of the Ethernet interface" );
            if ( ! _configureMacAddress() )
            {
                Log::error( "Failed to set Ethernet interface MAC address" );
                return;
            }
            esp_event_handler_register(
                ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_cb, this
            );    // register Ethernet event handler (to deal with user specific stuffs when event like link up/down
                  // happened)

            _attachTcpIpStack();
        }

        void start() { esp_eth_start( _ethHandle ); }

        // void getMacAddress( uint8_t * mac ) { memcpy( mac, _macAddress, ETH_ADDR_LEN ); }
        esp_netif_t * getNetif() { return _netif; }

        Runtime getStatus() { return _status; }

        Config getConfig() { return _config; }

        void setConfig( Config config )
        {
            _config = config;
            if ( _config.useDHCP )
            {
                _setDHCP();
            }
            else
            {
                _setStatic();
            }
        }

    private:
        static constexpr const char TAG[] = "W5500Ethernet";
        using Log                         = AsnPlus::Logger< ProjectConfig::LOG_LEVEL_ETHERNET, TAG >;

        gpio_num_t        _mosi, _miso, _sck, _cs, _nRst, _int;
        spi_host_device_t _spiHost;
        int               _clockSpeed;

        esp_netif_t *    _netif = nullptr;
        esp_eth_handle_t _ethHandle;

        Config  _config;
        Runtime _status;

        static void eth_event_cb( void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data )
        {
            ( (W5500Ethernet *) arg )->eth_event_handler( event_base, event_id, event_data );
        }

        static void eth_ip_event_cb( void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data )
        {
            ( (W5500Ethernet *) arg )->eth_ip_event_handler( event_base, event_id, event_data );
        }

        void eth_event_handler( esp_event_base_t event_base, int32_t event_id, void * event_data )
        {
            uint8_t mac_addr[ 6 ]       = { 0 };
            /* we can get the ethernet driver handle from event data */
            esp_eth_handle_t eth_handle = *(esp_eth_handle_t *) event_data;

            switch ( event_id )
            {
                case ETHERNET_EVENT_CONNECTED:
                    esp_eth_ioctl( eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr );
                    Log::info( "Ethernet Link Up" );
                    Log::info(
                        "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                        mac_addr[ 0 ],
                        mac_addr[ 1 ],
                        mac_addr[ 2 ],
                        mac_addr[ 3 ],
                        mac_addr[ 4 ],
                        mac_addr[ 5 ]
                    );
                    _status.linkUp = true;
                    break;
                case ETHERNET_EVENT_DISCONNECTED:
                    Log::info( "Ethernet Link Down" );
                    _status.linkUp  = false;
                    _status.ip      = IPAddress::empty();
                    _status.mask    = IPAddress::empty();
                    _status.gateway = IPAddress::empty();
                    break;
                case ETHERNET_EVENT_START:
                    Log::info( "Ethernet Started" );
                    break;
                case ETHERNET_EVENT_STOP:
                    Log::info( "Ethernet Stopped" );
                    break;
                default:
                    break;
            }
        }

        void eth_ip_event_handler( esp_event_base_t event_base, int32_t event_id, void * event_data )
        {
            // if "got ip" event, set Runtime
            switch ( event_id )
            {
                case IP_EVENT_ETH_GOT_IP:
                    {
                        ip_event_got_ip_t *         event   = (ip_event_got_ip_t *) event_data;
                        const esp_netif_ip_info_t * ip_info = &event->ip_info;

                        // Extract octets from uint32_t addresses
                        _status.ip                          = IPAddress(
                            ( ip_info->ip.addr >> 0 ) & 0xFF,
                            ( ip_info->ip.addr >> 8 ) & 0xFF,
                            ( ip_info->ip.addr >> 16 ) & 0xFF,
                            ( ip_info->ip.addr >> 24 ) & 0xFF
                        );
                        _status.mask = IPAddress(
                            ( ip_info->netmask.addr >> 0 ) & 0xFF,
                            ( ip_info->netmask.addr >> 8 ) & 0xFF,
                            ( ip_info->netmask.addr >> 16 ) & 0xFF,
                            ( ip_info->netmask.addr >> 24 ) & 0xFF
                        );
                        _status.gateway = IPAddress(
                            ( ip_info->gw.addr >> 0 ) & 0xFF,
                            ( ip_info->gw.addr >> 8 ) & 0xFF,
                            ( ip_info->gw.addr >> 16 ) & 0xFF,
                            ( ip_info->gw.addr >> 24 ) & 0xFF
                        );

                        Log::info( "Ethernet Got IP Address" );
                        Log::info( "IP: " IPSTR, IP2STR( &ip_info->ip ) );
                        Log::info( "Mask: " IPSTR, IP2STR( &ip_info->netmask ) );
                        Log::info( "Gateway: " IPSTR, IP2STR( &ip_info->gw ) );
                        break;
                    }
                case IP_EVENT_ETH_LOST_IP:
                    {
                        Log::info( "Ethernet Lost IP Address" );
                        _status.ip      = IPAddress( 0, 0, 0, 0 );
                        _status.mask    = IPAddress( 0, 0, 0, 0 );
                        _status.gateway = IPAddress( 0, 0, 0, 0 );
                        break;
                    }
            }
        }

        bool _initSpi()
        {
            spi_bus_config_t buscfg = {
                .mosi_io_num   = _mosi,
                .miso_io_num   = _miso,
                .sclk_io_num   = _sck,
                .quadwp_io_num = -1,
                .quadhd_io_num = -1,
            };

            spi_bus_initialize( _spiHost, &buscfg, SPI_DMA_CH_AUTO );
            return true;
        }

        bool _configureMacAddress()
        {
            uint8_t   baseMacAddress[ ETH_ADDR_LEN ] = { 0 };
            esp_err_t err                            = esp_efuse_mac_get_default( baseMacAddress );
            if ( err != ESP_OK )
            {
                Log::error( "Failed to retrieve efuse MAC. Reason: %d", err );
                return false;
            }
            uint8_t localMacAddress[ ETH_ADDR_LEN ] = { 0 };
            err                                     = esp_derive_local_mac( localMacAddress, baseMacAddress );
            if ( err != ESP_OK )
            {
                Log::error( "Failed to derive local MAC" );
                return false;
            }
            err = esp_eth_ioctl( _ethHandle, ETH_CMD_S_MAC_ADDR, localMacAddress );
            if ( err != ESP_OK )
            {
                Log::error( "Failed to set MAC on the interface" );
                return false;
            }
            memcpy( _status.mac, localMacAddress, ETH_ADDR_LEN );
            Log::info(
                "Ethernet MAC succesfully set to %02x:%02x:%02x:%02x:%02x:%02x",
                localMacAddress[ 0 ],
                localMacAddress[ 1 ],
                localMacAddress[ 2 ],
                localMacAddress[ 3 ],
                localMacAddress[ 4 ],
                localMacAddress[ 5 ]
            );
            return true;
        }

        bool _attachTcpIpStack()
        {
            Log::info( "Attaching Ethernet driver to TCP/IP stack" );
            esp_netif_config_t cfg =
                ESP_NETIF_DEFAULT_ETH();       // apply default network interface configuration for Ethernet
            _netif = esp_netif_new( &cfg );    // create network interface for Ethernet driver

            esp_netif_attach(
                _netif, esp_eth_new_netif_glue( _ethHandle )
            );    // attach Ethernet driver to TCP/IP stack
            esp_event_handler_register(
                IP_EVENT, IP_EVENT_ETH_GOT_IP, &eth_ip_event_cb, this
            );    // register user defined IP event handlers
            Log::info( "Ethernet driver attached to TCP/IP stack successfully" );
            return ESP_OK;
        }

        void _setDHCP()
        {
            if ( _netif == nullptr )
            {
                Log::error( "Cannot set DHCP: netif is not initialized" );
                return;
            }

            Log::info( "Configuring Ethernet interface for DHCP" );
            
            // Stop DHCP client if it's running
            esp_netif_dhcpc_stop( _netif );
            
            // Start DHCP client
            esp_err_t err = esp_netif_dhcpc_start( _netif );
            if ( err != ESP_OK && err != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STARTED )
            {
                Log::error( "Failed to start DHCP client: %d", err );
                return;
            }
            
            Log::info( "DHCP client started successfully" );
        }

        void _setStatic()
        {
            if ( _netif == nullptr )
            {
                Log::error( "Cannot set static IP: netif is not initialized" );
                return;
            }

            Log::info( "Configuring Ethernet interface for static IP" );
            
            // Stop DHCP client
            esp_err_t err = esp_netif_dhcpc_stop( _netif );
            if ( err != ESP_OK && err != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED )
            {
                Log::error( "Failed to stop DHCP client: %d", err );
                return;
            }
            
            // Configure static IP
            esp_netif_ip_info_t ip_info;
            ip_info.ip.addr      = _config.ip.toUint32();
            ip_info.netmask.addr = _config.mask.toUint32();
            ip_info.gw.addr      = _config.gateway.toUint32();
            
            err = esp_netif_set_ip_info( _netif, &ip_info );
            if ( err != ESP_OK )
            {
                Log::error( "Failed to set static IP address: %d", err );
                return;
            }
            
            Log::info( 
                "Static IP configured successfully: IP=%d.%d.%d.%d, Mask=%d.%d.%d.%d, Gateway=%d.%d.%d.%d",
                _config.ip.octets[0], _config.ip.octets[1], _config.ip.octets[2], _config.ip.octets[3],
                _config.mask.octets[0], _config.mask.octets[1], _config.mask.octets[2], _config.mask.octets[3],
                _config.gateway.octets[0], _config.gateway.octets[1], _config.gateway.octets[2], _config.gateway.octets[3]
            );
        }
    };
}    // namespace AsnPlus::Networking

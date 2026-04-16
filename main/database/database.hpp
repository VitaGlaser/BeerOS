#pragma once

#include "asn/asn-core/logger.hpp"

#include "asn/asn-hal/common/identification_structs.hpp"

#include "asn/asn-esp32-hal/old/nvs.hpp"

#include "asn/asn-esp32-ble/service/ota_service.hpp"
#include "asn/asn-esp32-ble/service/service.hpp"

#include "asn/asn-esp32-ble/attribute/array_attribute.hpp"
#include "asn/asn-esp32-ble/attribute/command_attribute.hpp"
#include "asn/asn-esp32-ble/attribute/simple_attribute.hpp"
#include "asn/asn-esp32-ble/attribute/string_attribute.hpp"
#include "asn/asn-esp32-ble/attribute/superstring_attribute.hpp"

#include "asn/asn-esp32-ble/adv_data.hpp"
#include "asn/asn-esp32-ble/structs.hpp"
#include "asn/asn-esp32-ble/uuid.hpp"

#include "asn/asn-esp32-ble/service/wifi_service.hpp"

#include "asn/asn-esp32-wifi/wifi_config.hpp"

#include "asn/asn-esp32-wifi/ethernet/ethernet.hpp"

#include "adv_data.hpp"

#include "program/config.hpp"

#include "components/connection/structs.hpp"

#include "components/measurement/channel.hpp"

#include "asn/asn-hal/time_manager/structs.hpp"

#include "components/cloud/json_conversion/device_config.hpp"
#include "components/cloud/json_conversion/network_config.hpp"

#include "components/measurement/event_monitor.hpp"
#include "components/psram_util.hpp"

namespace AsnPlus
{
    class Database
    {
    public:
        static const u8 SERVICE_COUNT               = 0;

        // MARK: Database version
        using DatabaseVersion                       = uint16_t;
        static constexpr DatabaseVersion DB_VERSION = 0;

        FirmwareInfo    versionInfo {};
        ManufactureInfo manufactureInfo {};
        OwnerInfo       ownerInfo {};

        AdvertisingData   advertisingData {};
        Bluetooth::Config bluetoothConfig {};
        Bluetooth::State  bluetoothState {};
        Wifi::WifiConfig  wifiConfig {};

        Network::W5500Sta::Runtime ethStaRuntime {};

        Network::W5500Ethernet::Config  ethConfig {};
        Network::W5500Ethernet::Runtime ethRuntime {};

        TimeConfig        timeConfig {};
        TimeRuntime       timeRuntime {};
        TimeChangeRequest timeChangeRequest {};

        Cloud::DeviceConfig  deviceConfig {};
        Cloud::NetworkConfig networkConfig {};

        // MARK: Sensor configs and runtimes
        Array< Channel::Config, DataSource::Manager::NUM_CHANNELS >  channelConfigs {};
        Array< Channel::Runtime, DataSource::Manager::NUM_CHANNELS > channelRuntimes {};

        // MARK: Event history — one PSRAM-allocated backing buffer and RingBufferExt per channel
        EventMonitor::Event * eventHistory0Buffer = static_cast< EventMonitor::Event * >( heap_caps_malloc(
            sizeof( EventMonitor::Event ) * EventMonitor::MAX_EVENT_HISTORY,
            MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
        ) );
        EventMonitor::Event * eventHistory1Buffer = static_cast< EventMonitor::Event * >( heap_caps_malloc(
            sizeof( EventMonitor::Event ) * EventMonitor::MAX_EVENT_HISTORY,
            MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
        ) );
        EventMonitor::Event * eventHistory2Buffer = static_cast< EventMonitor::Event * >( heap_caps_malloc(
            sizeof( EventMonitor::Event ) * EventMonitor::MAX_EVENT_HISTORY,
            MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
        ) );
        EventMonitor::Event * eventHistory3Buffer = static_cast< EventMonitor::Event * >( heap_caps_malloc(
            sizeof( EventMonitor::Event ) * EventMonitor::MAX_EVENT_HISTORY,
            MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
        ) );

        RingBufferExt< EventMonitor::Event > eventHistory0 { eventHistory0Buffer, EventMonitor::MAX_EVENT_HISTORY };
        RingBufferExt< EventMonitor::Event > eventHistory1 { eventHistory1Buffer, EventMonitor::MAX_EVENT_HISTORY };
        RingBufferExt< EventMonitor::Event > eventHistory2 { eventHistory2Buffer, EventMonitor::MAX_EVENT_HISTORY };
        RingBufferExt< EventMonitor::Event > eventHistory3 { eventHistory3Buffer, EventMonitor::MAX_EVENT_HISTORY };

        Connection::Config  connectionModuleConfig { 0, true, true, true, false, false };
        Connection::Runtime connectionModuleRuntime {};

        // Services - need to be instantiated before characteristics(attributes)
        Bluetooth::OtaService  otaService;
        Bluetooth::WifiService wifiService { wifiConfig };

        // MARK: NVS keys
        static constexpr const char DEVICE_CONFIG_NVS_KEY[]  = "device_cfg";
        static constexpr const char NETWORK_CONFIG_NVS_KEY[] = "network_cfg";
        static constexpr const char CH_CONFIG_NVS_KEY_FMT[]  = "ch_cfg_%u";

        void loadConfigs()
        {
            if ( Esp32::Nvs::load_config( deviceConfig, (char *) DEVICE_CONFIG_NVS_KEY ) != ESP_OK )
            {
                Log::error( "Failed to load deviceConfig from NVS" );
                saveDeviceConfig();    // Save default config if loading failed
            }

            if ( Esp32::Nvs::load_config( networkConfig, (char *) NETWORK_CONFIG_NVS_KEY ) != ESP_OK )
            {
                Log::error( "Failed to load networkConfig from NVS" );
                saveNetworkConfig();    // Save default config if loading failed
            }

            for ( uint8_t i = 0; i < DataSource::Manager::NUM_CHANNELS; ++i )
            {
                char key[ 16 ];
                snprintf( key, sizeof( key ), CH_CONFIG_NVS_KEY_FMT, i );
                if ( Esp32::Nvs::load_config( channelConfigs[ i ], key ) != ESP_OK )
                {
                    Log::error( "Failed to load channelConfigs[%u] from NVS", i );
                    saveChannelConfig( i );    // Save default config if loading failed
                }
            }
        }

        void saveDeviceConfig()
        {
            if ( Esp32::Nvs::store_config( deviceConfig, (char *) DEVICE_CONFIG_NVS_KEY ) != ESP_OK )
                Log::error( "Failed to save deviceConfig to NVS" );
        }

        void saveNetworkConfig()
        {
            if ( Esp32::Nvs::store_config( networkConfig, (char *) NETWORK_CONFIG_NVS_KEY ) != ESP_OK )
                Log::error( "Failed to save networkConfig to NVS" );
        }

        void saveChannelConfig( uint8_t index )
        {
            char key[ 16 ];
            snprintf( key, sizeof( key ), CH_CONFIG_NVS_KEY_FMT, index );
            if ( Esp32::Nvs::store_config( channelConfigs[ index ], key ) != ESP_OK )
                Log::error( "Failed to save channelConfigs[%u] to NVS", index );
        }

        void initialize()
        {
            Log::info( "Initializing" );
            manufactureInfo.environment = ManufactureInfo::Environment::DEVELOP;

            if ( ! eventHistory0Buffer || ! eventHistory1Buffer || ! eventHistory2Buffer || ! eventHistory3Buffer )
            {
                Log::error( "Failed to allocate event history buffers in PSRAM" );
            }

            loadConfigs();

            Log::info( "Initialized" );
        }

        void poll()
        {
            ethConfig = networkConfig.ethernetConfig;

            if ( networkConfig.wifiConfig.ssid[ 0 ] != '\0' &&
                 strcmp( wifiConfig.saved_networks[ 0 ].ssid.data, networkConfig.wifiConfig.ssid ) != 0 )
            {
                Log::info(
                    "Updating saved Wi-Fi credentials in database old: %s, new: %s",
                    wifiConfig.saved_networks[ 0 ].ssid.data,
                    networkConfig.wifiConfig.ssid
                );
                strncpy( wifiConfig.saved_networks[ 0 ].ssid.data, networkConfig.wifiConfig.ssid, Wifi::SSID_LENGTH );
                strncpy(
                    wifiConfig.saved_networks[ 0 ].password.data,
                    networkConfig.wifiConfig.password,
                    Wifi::PASSWORD_LENGTH
                );
                wifiConfig.store_saved_networks();
                wifiConfig.command = Wifi::Commands::DISCONNECT;
            }
        }

    private:
        static constexpr const char TAG[] = "Database";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL, TAG >;
    };

}    // namespace AsnPlus

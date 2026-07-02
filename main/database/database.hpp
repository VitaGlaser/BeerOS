#pragma once

#include "asn/asn-core/logger.hpp"

#include "asn/asn-hal/include/common/identification_structs.hpp"

#include "asn/asn-esp32-hal/old/nvs.hpp"

#include "asn/asn-esp32-ble/include/service/ota_service.hpp"
#include "asn/asn-esp32-ble/include/service/service.hpp"

#include "asn/asn-esp32-ble/include/attribute/array_attribute.hpp"
#include "asn/asn-esp32-ble/include/attribute/command_attribute.hpp"
#include "asn/asn-esp32-ble/include/attribute/simple_attribute.hpp"
#include "asn/asn-esp32-ble/include/attribute/string_attribute.hpp"
#include "asn/asn-esp32-ble/include/attribute/superstring_attribute.hpp"

#include "asn/asn-esp32-ble/include/adv_data.hpp"
#include "asn/asn-esp32-ble/include/structs.hpp"
#include "asn/asn-esp32-ble/include/uuid.hpp"

#include "asn/asn-esp32-ble/include/service/identification_service.hpp"
#include "asn/asn-esp32-ble/include/service/wifi_service.hpp"
#include "asn/asn-esp32-ble/include/service/time_service.hpp"

#include "asn/asn-esp32-hal/include/peripherals/persistent_storage.hpp"

#include "asn/asn-esp32-wifi/old/wifi_config.hpp"

#include "asn/asn-esp32-wifi/include/ethernet/ethernet.hpp"

#include "adv_data.hpp"

#include "program/config.hpp"

#include "components/connection/structs.hpp"

#include "components/measurement/channel.hpp"

#include "asn/asn-hal/include/time_manager/structs.hpp"

#include "asn/asn-esp32-wifi/include/mqtt/client.hpp"

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

        Esp32::PersistentStorage & persistentStorage;

        FirmwareInfo    versionInfo {};
        ManufactureInfo manufactureInfo {
            .uid         = { 'R', 'V', 'S', 'S', 'B', 'H', 'M', 'B', 'R', 'A', '0', '0', '0', '0', '0', '0' },
            .environment = ManufactureInfo::Environment::DEVELOP
        };
        OwnerInfo ownerInfo {};

        AdvertisingData   advertisingData {};
        Bluetooth::Config bluetoothConfig {};
        Bluetooth::State  bluetoothState {};
        Wifi::LegacyWifiConfig  wifiConfig {};
        uint8_t                 activeWifiSlotIndex = 0;

        Network::W5500Sta::Runtime ethStaRuntime {};

        Network::W5500Ethernet::Config  ethConfig {};
        Network::W5500Ethernet::Runtime ethRuntime {};

        TimeConfig        timeConfig {};
        TimeRuntime       timeRuntime {};
        TimeChangeRequest timeChangeRequest {};

        Esp32::Mqtt::Client::Config mqttConfig { .enabled = false };

        Cloud::DeviceConfig  deviceConfig {};
        Cloud::NetworkConfig networkConfig {};

        // MARK: Sensor configs and runtimes
        Array< Channel::Config, DataSource::Manager::NUM_CHANNELS >  channelConfigs {};
        Array< Channel::Runtime, DataSource::Manager::NUM_CHANNELS > channelRuntimes {};
        Array< uint32_t, DataSource::Manager::NUM_CHANNELS >         channelHistorySeqNums {};

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

        uint32_t uptime = 0;

        Connection::Config  connectionModuleConfig { 0, true, true, true, false, false };
        Connection::Runtime connectionModuleRuntime {};

        // Services - need to be instantiated before characteristics(attributes)
        Bluetooth::OtaService  otaService;
        Bluetooth::IdentificationService
        identificationService { persistentStorage, versionInfo, manufactureInfo, ownerInfo };
        Bluetooth::TimeService timeService { timeConfig, timeRuntime, timeChangeRequest };
        Bluetooth::WifiService wifiService { wifiConfig };

        // MARK: NVS keys
        static constexpr const char DEVICE_CONFIG_NVS_KEY[]  = "device_cfg";
        static constexpr const char NETWORK_CONFIG_NVS_KEY[] = "network_cfg";
        static constexpr const char MQTT_CONFIG_NVS_KEY[]    = "mqtt_cfg";
        static constexpr const char ACTIVE_WIFI_SLOT_NVS_KEY[] = "act_wifi_slot";
        static constexpr const char CH_CONFIG_NVS_KEY_FMT[]  = "ch_cfg_%u";
        static constexpr const char CH_SEQ_NVS_KEY_FMT[]     = "ch_seq_%u";

        Database( Esp32::PersistentStorage & persistentStorage ) : persistentStorage( persistentStorage ) {}

        void initialize()
        {
            Log::info( "Initializing" );

            timeService.initialize();
            identificationService.initialize();

            memcpy(
                advertisingData.manufacturer_specific.serial_number, manufactureInfo.uid, ManufactureInfo::UID_LENGTH
            );

            if ( ! eventHistory0Buffer || ! eventHistory1Buffer || ! eventHistory2Buffer || ! eventHistory3Buffer )
            {
                Log::error( "Failed to allocate event history buffers in PSRAM" );
            }

            loadConfigs();

            char uidStr[ ManufactureInfo::UID_LENGTH + 1 ] = {};
            memcpy( uidStr, manufactureInfo.uid, ManufactureInfo::UID_LENGTH );
            strncpy( mqttConfig.clientId, uidStr, sizeof( mqttConfig.clientId ) - 1 );            

            Log::info( "Initialized" );
        }

        void poll()
        {
            timeService.poll();

            if ( activeWifiSlotIndex != 0 )
            {
                Log::warn( "Forcing active Wi-Fi slot to 0 (was %u)", activeWifiSlotIndex );
                activeWifiSlotIndex = 0;
            }

            if ( activeWifiSlotIndex != _lastSavedActiveWifiSlotIndex )
            {
                Log::info(
                    "Persisting active Wi-Fi slot change: %u -> %u",
                    _lastSavedActiveWifiSlotIndex,
                    activeWifiSlotIndex
                );
                saveActiveWifiSlotIndex();
                _lastSavedActiveWifiSlotIndex = activeWifiSlotIndex;
            }

            uptime    = static_cast< uint32_t >( Utils::getMs64() / 1000ULL );

            ethConfig = networkConfig.ethernetConfig;

            const bool networkConfigChanged = ( networkConfig.timestamp != _lastNetworkConfigTimestamp );
            if ( networkConfigChanged )
            {
                _applyNetworkConfigToSlot0();
                _lastNetworkConfigTimestamp = networkConfig.timestamp;
            }
            else
            {
                _syncSlot0ToNetworkConfig();
            }

            _snapshotSlot0Credentials();
        }

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

            if ( Esp32::Nvs::load_config( mqttConfig, (char *) MQTT_CONFIG_NVS_KEY ) != ESP_OK )
            {
                Log::error( "Failed to load mqttConfig from NVS" );
                saveMqttConfig();    // Save default config if loading failed
            }

            if ( Esp32::Nvs::load_config( activeWifiSlotIndex, (char *) ACTIVE_WIFI_SLOT_NVS_KEY ) != ESP_OK )
            {
                Log::warn( "No saved active Wi-Fi slot, using slot 0" );
                activeWifiSlotIndex = 0;
                saveActiveWifiSlotIndex();
            }

            if ( activeWifiSlotIndex >= Wifi::LegacyWifiConfig::MAX_SAVED )
            {
                Log::warn( "Invalid active Wi-Fi slot %u, resetting to 0", activeWifiSlotIndex );
                activeWifiSlotIndex = 0;
                saveActiveWifiSlotIndex();
            }

            if ( activeWifiSlotIndex != 0 )
            {
                Log::warn( "Wi-Fi is configured to use slot0 only, resetting active slot from %u to 0", activeWifiSlotIndex );
                activeWifiSlotIndex = 0;
                saveActiveWifiSlotIndex();
            }

            _lastSavedActiveWifiSlotIndex = activeWifiSlotIndex;
            _lastNetworkConfigTimestamp   = networkConfig.timestamp;
            _snapshotSlot0Credentials();

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

            for ( uint8_t i = 0; i < DataSource::Manager::NUM_CHANNELS; ++i )
            {
                char key[ 16 ];
                snprintf( key, sizeof( key ), CH_SEQ_NVS_KEY_FMT, i );
                if ( Esp32::Nvs::load_config( channelHistorySeqNums[ i ], key ) != ESP_OK )
                    Log::warn( "No saved sequence number for channel %u, starting from 0", i );
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

        void saveMqttConfig()
        {
            if ( Esp32::Nvs::store_config( mqttConfig, (char *) MQTT_CONFIG_NVS_KEY ) != ESP_OK )
                Log::error( "Failed to save mqttConfig to NVS" );
        }

        void saveActiveWifiSlotIndex()
        {
            if ( Esp32::Nvs::store_config( activeWifiSlotIndex, (char *) ACTIVE_WIFI_SLOT_NVS_KEY ) != ESP_OK )
                Log::error( "Failed to save active Wi-Fi slot index to NVS" );
        }

        void saveChannelConfig( uint8_t index )
        {
            char key[ 16 ];
            snprintf( key, sizeof( key ), CH_CONFIG_NVS_KEY_FMT, index );
            if ( Esp32::Nvs::store_config( channelConfigs[ index ], key ) != ESP_OK )
                Log::error( "Failed to save channelConfigs[%u] to NVS", index );
        }

        void saveChannelHistorySeqNum( uint8_t index )
        {
            char key[ 16 ];
            snprintf( key, sizeof( key ), CH_SEQ_NVS_KEY_FMT, index );
            if ( Esp32::Nvs::store_config( channelHistorySeqNums[ index ], key ) != ESP_OK )
                Log::error( "Failed to save channelHistorySeqNums[%u] to NVS", index );
        }

    private:
        static constexpr const char TAG[] = "Database";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL, TAG >;
        uint8_t _lastSavedActiveWifiSlotIndex = 0;
        uint64_t _lastNetworkConfigTimestamp   = 0;
        char _lastSlot0Ssid[ Wifi::SSID_LENGTH + 1 ] = {};
        char _lastSlot0Password[ Wifi::PASSWORD_LENGTH + 1 ] = {};

        uint64_t _getConfigTimestampNow() const
        {
            if ( timeRuntime.utcEpochMs > 0 ) return timeRuntime.utcEpochMs;
            return Utils::getMs64();
        }

        void _snapshotSlot0Credentials()
        {
            wifiConfig.saved_networks[ 0 ].ssid.terminate();
            wifiConfig.saved_networks[ 0 ].password.terminate();

            strncpy( _lastSlot0Ssid, wifiConfig.saved_networks[ 0 ].ssid.data, Wifi::SSID_LENGTH );
            _lastSlot0Ssid[ Wifi::SSID_LENGTH ] = '\0';

            strncpy( _lastSlot0Password, wifiConfig.saved_networks[ 0 ].password.data, Wifi::PASSWORD_LENGTH );
            _lastSlot0Password[ Wifi::PASSWORD_LENGTH ] = '\0';
        }

        bool _slot0DiffersFromNetworkConfig() const
        {
            return (
                strncmp( wifiConfig.saved_networks[ 0 ].ssid.data, networkConfig.wifiConfig.ssid, Wifi::SSID_LENGTH ) != 0 ||
                strncmp( wifiConfig.saved_networks[ 0 ].password.data, networkConfig.wifiConfig.password, Wifi::PASSWORD_LENGTH ) != 0
            );
        }

        bool _slot0ChangedSinceLastSnapshot() const
        {
            return (
                strncmp( wifiConfig.saved_networks[ 0 ].ssid.data, _lastSlot0Ssid, Wifi::SSID_LENGTH ) != 0 ||
                strncmp( wifiConfig.saved_networks[ 0 ].password.data, _lastSlot0Password, Wifi::PASSWORD_LENGTH ) != 0
            );
        }

        void _applyNetworkConfigToSlot0()
        {
            wifiConfig.saved_networks[ 0 ].ssid.terminate();
            wifiConfig.saved_networks[ 0 ].password.terminate();

            if ( ! _slot0DiffersFromNetworkConfig() ) return;

            Log::info(
                "Applying networkConfig Wi-Fi to slot0 old: %s, new: %s",
                wifiConfig.saved_networks[ 0 ].ssid.data,
                networkConfig.wifiConfig.ssid
            );

            strncpy( wifiConfig.saved_networks[ 0 ].ssid.data, networkConfig.wifiConfig.ssid, Wifi::SSID_LENGTH );
            wifiConfig.saved_networks[ 0 ].ssid.data[ Wifi::SSID_LENGTH - 1 ] = '\0';

            strncpy( wifiConfig.saved_networks[ 0 ].password.data, networkConfig.wifiConfig.password, Wifi::PASSWORD_LENGTH );
            wifiConfig.saved_networks[ 0 ].password.data[ Wifi::PASSWORD_LENGTH - 1 ] = '\0';

            wifiConfig.store_saved_networks();
            wifiConfig.command = Wifi::Commands::DISCONNECT;

            networkConfig.timestamp   = _getConfigTimestampNow();
            _lastNetworkConfigTimestamp = networkConfig.timestamp;
            saveNetworkConfig();
        }

        void _syncSlot0ToNetworkConfig()
        {
            wifiConfig.saved_networks[ 0 ].ssid.terminate();
            wifiConfig.saved_networks[ 0 ].password.terminate();

            if ( ! _slot0ChangedSinceLastSnapshot() ) return;

            Log::info( "Detected local slot0 Wi-Fi update, syncing to networkConfig" );

            strncpy( networkConfig.wifiConfig.ssid, wifiConfig.saved_networks[ 0 ].ssid.data, Wifi::SSID_LENGTH );
            networkConfig.wifiConfig.ssid[ Wifi::SSID_LENGTH - 1 ] = '\0';

            strncpy( networkConfig.wifiConfig.password, wifiConfig.saved_networks[ 0 ].password.data, Wifi::PASSWORD_LENGTH );
            networkConfig.wifiConfig.password[ Wifi::PASSWORD_LENGTH - 1 ] = '\0';

            networkConfig.timestamp   = _getConfigTimestampNow();
            _lastNetworkConfigTimestamp = networkConfig.timestamp;
            saveNetworkConfig();
        }
    };

}    // namespace AsnPlus

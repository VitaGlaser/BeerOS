#pragma once

#include "asn_module_config.hpp"

#include "freertos/FreeRTOS.h"

#include "esp_crt_bundle.h"
#include "esp_https_ota.h"
#include "esp_system.h"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/timer.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/utils.hpp"

#include "asn/asn-esp32-hal/include/peripherals/gpio.hpp"
#include "asn/asn-esp32-hal/include/peripherals/i2c_master.hpp"
#include "asn/asn-esp32-hal/include/peripherals/spi_master.hpp"
#include "asn/asn-esp32-hal/include/peripherals/uart.hpp"

#include "asn/asn-drivers/include/pcf85263.hpp"

#include "asn/asn-expander-lib/include/adc/adc.hpp"
#include "asn/asn-expander-lib/include/expander.hpp"
#include "asn/asn-expander-lib/include/gateway/i2c_gateway.hpp"
#include "asn/asn-expander-lib/include/hal/gpio.hpp"
#include "asn/asn-expander-lib/include/spi_transport.hpp"

#include "asn/asn-esp32-ble/include/nimble.hpp"

#include "asn/asn-esp32-wifi/include/ethernet/ethernet.hpp"
#include "asn/asn-esp32-wifi/include/ethernet/w5500_sta.hpp"
#include "components/wifi/manager_adapter.hpp"

#include "asn/asn-esp32-modbus/include/master.hpp"

#include "asn/asn-eg915-driver/include/eg915.hpp"
#include "asn/asn-eg915-driver/include/https/https_client.hpp"

#include "components/connection/manager.hpp"
#include "components/mqtt/manager.hpp"
#include "components/websocket/manager.hpp"

#include "components/cloud/request_manager.hpp"
#include "components/leds/ws2812_led_strip.hpp"

#include "components/measurement/channel_manager.hpp"
#include "components/measurement/data_sources/manager.hpp"
#include "components/measurement/history_manager.hpp"

#include "asn/asn-esp32-hal/include/time_manager/system_clock.hpp"

#include "asn/asn-esp32-hal/include/peripherals/persistent_storage.hpp"

#include "asn/asn-hal/include/time_manager/time_manager.hpp"

#include "database/database.hpp"

#include "pinout.hpp"

namespace AsnPlus
{
    class Components
    {
    public:
        Esp32::Nvs nvs {};

        Esp32::PersistentStorage::Config persistentStorageConfig {};
        Esp32::PersistentStorage         persistentStorage { persistentStorageConfig };

        Database database { persistentStorage };

        // MARK: Time related stuff
        Esp32::I2cMaster::Config i2cConfig { 100'000, I2C_NUM_0, Pinout::I2C_SDA, Pinout::I2C_SCL, true };
        Esp32::I2cMaster         i2cMaster { i2cConfig };
        Drivers::Pcf85263a       pcfRtc { i2cMaster };

        Esp32::SystemClock systemClock {};

        TimeManager timeManager {
            systemClock,
            &pcfRtc,
            persistentStorage,
            database.timeConfig,
            database.timeRuntime,
            database.timeChangeRequest
        };

        // MARK: Communication
        Bluetooth::Nimble nimble { database.advertisingData, database.bluetoothConfig, database.bluetoothState };

        Wifi::WifiManager    wifiManager { database.wifiConfig };
        Esp32::Https::Client wifiClient {};

        // LTE
        Esp32::Gpio::Config lteResetConfig {
            IGpio::Config::PinMode::OUTPUT,
            IGpio::Config::InterruptType::NONE,
            Pinout::LTE_RESET,
            false,
            false
        };
        Esp32::Gpio lteReset { lteResetConfig };

        Esp32::Uart::Config _lteUartConfig { 115'200, UART_NUM_2, Pinout::LTE_UART_TX, Pinout::LTE_UART_RX };
        Esp32::Uart         lteUart { _lteUartConfig };
        AtUart              lteAtUart { lteUart };
        Eg915               lte { lteAtUart };
        Eg915HttpsClient    lteClient { lte };

        Network::W5500Sta::Config ethStaConfig {
            .spiHost       = SPI2_HOST,
            .mosi          = Pinout::ETH_MOSI,
            .miso          = Pinout::ETH_MISO,
            .sck           = Pinout::ETH_SCK,
            .cs            = Pinout::ETH_CS,
            .nRst          = Pinout::ETH_nRST,
            .intPin        = Pinout::ETH_nINT,
            .clockSpeedMHz = 20,
        };

        Network::W5500Sta      ethSta { ethStaConfig, database.ethStaRuntime };
        Network::W5500Ethernet ethernet { database.ethConfig, database.ethRuntime, ethSta };
        Esp32::Https::Client   ethernetClient {};

        Esp32::Mqtt::Client::Runtime mqttRuntime {};
        Esp32::Mqtt::Client          mqttClient {
            database.mqttConfig,
            mqttRuntime,
            Esp32::Mqtt::Client::OnMessage::create< Mqtt::Manager, &Mqtt::Manager::onMessage >( mqttManager ),
            Esp32::Mqtt::Client::OnConnect::create< Mqtt::Manager, &Mqtt::Manager::onConnected >( mqttManager ),
        };

        Mqtt::Manager mqttManager { mqttClient, database };
        Websocket::Manager websocketManager { database };

        Cloud::RequestManager requestManager { database };

        Connection::Manager connectionManager {
            database.connectionModuleConfig,
            database.connectionModuleRuntime,
            database,
            database.bluetoothState,
            nimble,
            ethernet,
            ethernetClient,
            database.wifiConfig,
            wifiManager,
            wifiClient,
            lte,
            lteClient,
            requestManager,
            mqttManager,
            websocketManager
        };

        // MARK: Peripheral Ports
        Modbus::RtuMaster::Config modbusConfig { 9600, UART_PARITY_EVEN, UART_DATA_8_BITS, UART_STOP_BITS_1, 100 };
        Modbus::RtuMaster
            rtuMaster { modbusConfig, UART_NUM_1, Pinout::RTU_TERM_TX, Pinout::RTU_TERM_RX, Pinout::RTU_TERM_DIR };

        // MARK: LED strip
        Drivers::LedStrip ledStrip { Pinout::LED_DATA, 16, false };

        // MARK: Expander
        Esp32::Gpio::Config expanderNrstConfig {
            IGpio::Config::PinMode::OUTPUT,
            IGpio::Config::InterruptType::NONE,
            Pinout::EXPANDER_NRST,
            false,
            false
        };
        Esp32::Gpio expanderNrst { expanderNrstConfig };

        Esp32::Gpio::Config expanderCsConfig {
            IGpio::Config::PinMode::OUTPUT,
            IGpio::Config::InterruptType::NONE,
            Pinout::EXPANDER_CS,
            false,
            false
        };
        Esp32::Gpio expanderCs { expanderCsConfig };

        Esp32::SpiMaster::Config expanderSpiConfig {
            { 100'000 },
            SPI3_HOST,
            Pinout::EXPANDER_SPI_SCK,
            Pinout::EXPANDER_SPI_MOSI,
            Pinout::EXPANDER_SPI_MISO
        };
        Esp32::SpiMaster expanderSpi { expanderSpiConfig };

        Esp32::Gpio::Config expanderBootConfig {
            IGpio::Config::PinMode::OUTPUT,
            IGpio::Config::InterruptType::NONE,
            Pinout::EXPANDER_BOOT,
            false,
            false
        };
        Esp32::Gpio expanderBoot { expanderBootConfig };

        Expander::SpiTransport transport { expanderSpi, Expander::SpiTransport::DEFAULT_SLAVE_ID };
        Expander::Expander
            expander { transport, Expander::Expander::UpdateStrategy::BootPin, &expanderNrst, &expanderBoot };

        Expander::PortPinGpio::Config expanderGpioConfig {
            IGpio::Config::PinMode::OUTPUT,
            IGpio::Config::InterruptType::NONE
        };
        Expander::PortPinGpio ltePower { expanderGpioConfig, expander.getPortB().pin( Pinout::LTE_POWER_KEY ) };

        Expander::Adc::Adc          adc { expander.getAdc() };
        Expander::Adc::Adc::Channel adcChannel1 { adc.channel( 0 ) };
        Expander::Adc::Adc::Channel adcChannel2 { adc.channel( 1 ) };
        Expander::Adc::Adc::Channel adcChannel3 { adc.channel( 2 ) };
        Expander::Adc::Adc::Channel adcChannel4 { adc.channel( 3 ) };

        DataSource::Manager::AdcChannels adcChannels { adcChannel1, adcChannel2, adcChannel3, adcChannel4 };

        Expander::Timers::Timer &        timer1 { expander.getTimerD() };
        Expander::Timers::Timer::Channel timer1Channel1 { timer1, 0 };
        Expander::Timers::Timer &        timer2 { expander.getTimerB() };
        Expander::Timers::Timer::Channel timer2Channel1 { timer2, 0 };
        Expander::Timers::Timer &        timer3 { expander.getTimerC() };
        Expander::Timers::Timer::Channel timer3Channel1 { timer3, 0 };
        Expander::Timers::Timer &        timer4 { expander.getTimerE() };
        Expander::Timers::Timer::Channel timer4Channel1 { timer4, 0 };

        DataSource::Manager::PulseChannels
            pulseChannels { timer1Channel1, timer2Channel1, timer3Channel1, timer4Channel1 };

        // MARK: Application logic
        DataSource::Manager dataSourceManager { rtuMaster, pulseChannels, adcChannels };
        ChannelManager      channelManager { dataSourceManager, database };

        Vector< IRingBuffer< EventMonitor::Event > *, 4 > _eventHistories {
            &database.eventHistory0,
            &database.eventHistory1,
            &database.eventHistory2,
            &database.eventHistory3
        };
        HistoryManager historyManager { database.timeRuntime, _eventHistories };

        void initialize()
        {
            Log::info( "Initializing" );

            nvs.initialize();

            database.initialize();

            i2cMaster.initialize();
            timeManager.initialize();

            lteUart.initialize();
            _initializeLteTask();

            _initializeExpander();
            expander.poll();

            lte.setPowerKeyGpio( ltePower );
            connectionManager.initialize();

            rtuMaster.initialize();

            dataSourceManager.initialize();
            channelManager.initialize();
            historyManager.initialize();

            ledStrip.initialize();
            for ( int i = 0; i < 16; i += 2 ) ledStrip.setColor( i + 1, Color { 77, 0, 255 } );
            ledStrip.poll();
            Utils::delay( 1000 );
            ledStrip.clear_all();
            ledStrip.poll();

            _initializeTasks();

            Log::info( "Initialized" );

            if ( ProjectConfig::LOG_LEVEL >= 2 )
            {
                vTaskDelete( NULL );
            }
        }

        void poll()
        {

            Log::info(
                "Free heap: %lu %u %u",
                esp_get_free_heap_size(),
                heap_caps_get_largest_free_block( MALLOC_CAP_INTERNAL ),
                heap_caps_get_free_size( MALLOC_CAP_INTERNAL )
            );

            Utils::delay( 2000 );
        }

        void systemPoll()
        {
            timeManager.poll();
            historyManager.poll();
        }

        void componentsPoll()
        {
            ledStrip.poll();
            expander.poll();
        }

        void dataSourcePoll() { dataSourceManager.poll(); }

        void controlPoll() { channelManager.poll(); }

        void communicationPoll()
        {
            database.poll();
            connectionManager.poll();
            connectionManager.httpsPoll();
        }

        void streamPoll()
        {
            connectionManager.websocketPoll();

            // MQTT is controlled by mqttConfig.enabled from settings/cloud config.
            // Keeping this poll active allows runtime enable/disable without restart.
            connectionManager.mqttPoll();
        }

        void otaPoll()
        {
            if ( _otaInProgress ) return;

            Cloud::RequestManager::OtaJob job {};
            if ( ! requestManager.tryDequeueOtaJob( job ) ) return;

            _otaInProgress = true;

            requestManager.setReportedOtaStatus( "updating", job.version );
            requestManager.sendUnitStatusNow();
            Log::warn(
                "Starting OTA from status endpoint: version=%s mandatory=%s url=%s",
                job.version,
                job.mandatory ? "true" : "false",
                job.url
            );

            bool success = _performOtaUpdate( job );
            if ( success )
            {
                requestManager.setReportedOtaStatus( "rebooting", job.version );
                requestManager.sendUnitStatusNow();
                Utils::delay( 1500 );
                esp_restart();
            }
            else
            {
                requestManager.setReportedOtaStatus( "failed", job.version );
                requestManager.sendUnitStatusNow();
                _otaInProgress = false;
            }
        }

        void lteUartPoll() { lteAtUart.poll(); }

        static void systemTask( void * pvParameters )
        {
            TickType_t   _lastTaskTime = xTaskGetTickCount();
            Components * instance      = static_cast< Components * >( pvParameters );
            TickType_t   _lastTime     = xTaskGetTickCount();
            while ( true )
            {
                if ( xTaskGetTickCount() - _lastTime > SYSTEM_TASK_DELAY_MS )
                    ESP_LOGI( "systemTask", "Last time: %lu", xTaskGetTickCount() - _lastTime );
                _lastTime = xTaskGetTickCount();
                instance->systemPoll();
                vTaskDelayUntil( &_lastTaskTime, pdMS_TO_TICKS( SYSTEM_TASK_DELAY_MS ) );
            }
        }

        static void componentsTask( void * pvParameters )
        {
            TickType_t   _lastTaskTime = xTaskGetTickCount();
            Components * instance      = static_cast< Components * >( pvParameters );
            TickType_t   _lastTime     = xTaskGetTickCount();
            while ( true )
            {
                if ( xTaskGetTickCount() - _lastTime > COMPONENTS_TASK_DELAY_MS )
                    ESP_LOGD( "componentsTask", "Last time: %lu", xTaskGetTickCount() - _lastTime );
                _lastTime = xTaskGetTickCount();
                instance->componentsPoll();
                vTaskDelayUntil( &_lastTaskTime, pdMS_TO_TICKS( COMPONENTS_TASK_DELAY_MS ) );
            }
        }

        static void dataSourceTask( void * pvParameters )
        {
            Components * instance = static_cast< Components * >( pvParameters );
            while ( true )
            {
                instance->dataSourcePoll();
                vTaskDelay( pdMS_TO_TICKS( DATA_SOURCE_TASK_DELAY_MS ) );
            }
        }

        static void controlTask( void * pvParameters )
        {
            TickType_t   _lastTaskTime = xTaskGetTickCount();
            Components * instance      = static_cast< Components * >( pvParameters );
            TickType_t   _lastTime     = xTaskGetTickCount();
            while ( true )
            {
                if ( xTaskGetTickCount() - _lastTime > CONTROL_TASK_DELAY_MS )
                    ESP_LOGI( "controlTask", "Last time: %lu", xTaskGetTickCount() - _lastTime );
                _lastTime = xTaskGetTickCount();
                instance->controlPoll();
                vTaskDelayUntil( &_lastTaskTime, pdMS_TO_TICKS( CONTROL_TASK_DELAY_MS ) );
            }
        }

        static void communicationTask( void * pvParameters )
        {
            Components * components = static_cast< Components * >( pvParameters );
            while ( true )
            {
                components->communicationPoll();
                vTaskDelay( pdMS_TO_TICKS( COMMUNICATION_TASK_DELAY_MS ) );
            }
        }

        static void lteUartTask( void * pvParameters )
        {
            Components * components = static_cast< Components * >( pvParameters );
            while ( true )
            {
                components->lteUartPoll();
                vTaskDelay( pdMS_TO_TICKS( LTE_TASK_DELAY_MS ) );
            }
        }

        static void streamTask( void * pvParameters )
        {
            Components * components = static_cast< Components * >( pvParameters );
            while ( true )
            {
                components->streamPoll();
                vTaskDelay( pdMS_TO_TICKS( STREAM_TASK_DELAY_MS ) );
            }
        }

        static void otaTask( void * pvParameters )
        {
            Components * components = static_cast< Components * >( pvParameters );
            while ( true )
            {
                components->otaPoll();
                vTaskDelay( pdMS_TO_TICKS( OTA_TASK_DELAY_MS ) );
            }
        }

    private:
        static constexpr const char TAG[]                     = "Components";
        using Log                                             = AsnPlus::Logger< ProjectConfig::LOG_LEVEL, TAG >;

        static constexpr uint32_t COMMUNICATION_TASK_PRIORITY = 3;
        static constexpr uint32_t COMPONENTS_TASK_PRIORITY    = 4;
        static constexpr uint32_t CONTROL_TASK_PRIORITY       = 5;
        static constexpr uint32_t DATA_SOURCE_TASK_PRIORITY   = 4;
        static constexpr uint32_t LTE_TASK_PRIORITY           = 3;
        static constexpr uint32_t OTA_TASK_PRIORITY           = 1;
        static constexpr uint32_t STREAM_TASK_PRIORITY        = 4;
        static constexpr uint32_t SYSTEM_TASK_PRIORITY        = 2;

        static constexpr uint32_t COMMUNICATION_TASK_DELAY_MS = 100;
        static constexpr uint32_t COMPONENTS_TASK_DELAY_MS    = 25;
        static constexpr uint32_t CONTROL_TASK_DELAY_MS       = 10;
        static constexpr uint32_t DATA_SOURCE_TASK_DELAY_MS   = 25;
        static constexpr uint32_t LTE_TASK_DELAY_MS           = 100;
        static constexpr uint32_t OTA_TASK_DELAY_MS           = 1000;
        static constexpr uint32_t STREAM_TASK_DELAY_MS        = 200;
        static constexpr uint32_t SYSTEM_TASK_DELAY_MS        = 1000;

        bool _otaInProgress = false;

        bool _performOtaUpdate( const Cloud::RequestManager::OtaJob & job )
        {
            char fullUrl[ 256 ] = {};
            if ( job.url[ 0 ] == '/' )
            {
                snprintf( fullUrl, sizeof( fullUrl ), "https://beeros.revosoft.cz%s", job.url );
            }
            else
            {
                snprintf( fullUrl, sizeof( fullUrl ), "%s", job.url );
            }

            esp_http_client_config_t httpConfig {};
            httpConfig.url               = fullUrl;
            httpConfig.timeout_ms        = 15000;
            httpConfig.keep_alive_enable = true;
            httpConfig.crt_bundle_attach = esp_crt_bundle_attach;

            esp_https_ota_config_t otaConfig {};
            otaConfig.http_config = &httpConfig;

            esp_err_t result      = esp_https_ota( &otaConfig );
            if ( result != ESP_OK )
            {
                Log::error( "OTA failed for url=%s (%s)", fullUrl, esp_err_to_name( result ) );
                return false;
            }

            Log::warn( "OTA image applied successfully, rebooting" );
            return true;
        }

        void _initializeExpander()
        {
            expanderNrst.initialize();
            expanderBoot.initialize();

            expanderCs.initialize();
            expanderSpi.initialize();
            expanderSpi.addDevice( Expander::SpiTransport::DEFAULT_SLAVE_ID, expanderCs );

            transport.initialize();
            expander.initialize();

            uint8_t  major, minor;
            uint16_t patch;
            expander.getSystem().getVersion( major, minor, patch );
            Log::info( "Expander firmware version: %d.%d.%d", major, minor, patch );

            ltePower.initialize();

            // expander.getPortA().enable();
            expander.getPortB().enable();

            expander.getAdc().setSamplingFrequency( Expander::Adc::ADCCTRL::SamplingFrequency::SMP1 );

            adcChannel1.setPhysicalChannel( Expander::Adc::ADCMxCFG::ChannelSelect::CH1 );
            adcChannel1.setFilter( Expander::Adc::ADCMxCFG::LPFrequency::LPF20K );
            adcChannel2.setPhysicalChannel( Expander::Adc::ADCMxCFG::ChannelSelect::CH2 );
            adcChannel2.setFilter( Expander::Adc::ADCMxCFG::LPFrequency::LPF20K );
            adcChannel3.setPhysicalChannel( Expander::Adc::ADCMxCFG::ChannelSelect::CH3 );
            adcChannel3.setFilter( Expander::Adc::ADCMxCFG::LPFrequency::LPF20K );
            adcChannel4.setPhysicalChannel( Expander::Adc::ADCMxCFG::ChannelSelect::CH8 );
            adcChannel4.setFilter( Expander::Adc::ADCMxCFG::LPFrequency::LPF20K );

            expander.getAdc().startCalibration();
            expander.getAdc().waitCalibration();
            expander.getAdc().enable();

            _initializeTimers();
        }

        void _initializeTimers()
        {
            static constexpr uint16_t PRESCALER = 1999;
            static constexpr uint8_t  FILTER    = 15;

            timer1.setMode( Expander::TIMxCR::Mode::COUNT );
            timer1.setPrescaler( PRESCALER );
            timer1.setAutoReload( 0xFFFF );
            timer1.setAutoReloadPreload( true );

            timer1Channel1.setPolarity( Expander::TIMxCCMR::Polarity::BOTH_EDGES );
            timer1Channel1.setICPrescaler( Expander::TIMxCCMR::ICPrescaler::EVERY_EDGE );
            timer1Channel1.setICFilter( FILTER );
            timer1Channel1.enable();

            timer1.enable();

            timer2.setMode( Expander::TIMxCR::Mode::COUNT );
            timer2.setPrescaler( PRESCALER );
            timer2.setAutoReload( 0xFFFF );
            timer2.setAutoReloadPreload( true );

            timer2Channel1.setPolarity( Expander::TIMxCCMR::Polarity::BOTH_EDGES );
            timer2Channel1.setICPrescaler( Expander::TIMxCCMR::ICPrescaler::EVERY_EDGE );
            timer2Channel1.setICFilter( FILTER );
            timer2Channel1.enable();

            timer2.enable();

            timer3.setMode( Expander::TIMxCR::Mode::COUNT );
            timer3.setPrescaler( PRESCALER );
            timer3.setAutoReload( 0xFFFF );
            timer3.setAutoReloadPreload( true );

            timer3Channel1.setPolarity( Expander::TIMxCCMR::Polarity::BOTH_EDGES );
            timer3Channel1.setICPrescaler( Expander::TIMxCCMR::ICPrescaler::EVERY_EDGE );
            timer3Channel1.setICFilter( FILTER );
            timer3Channel1.enable();

            timer3.enable();

            timer4.setMode( Expander::TIMxCR::Mode::COUNT );
            timer4.setPrescaler( PRESCALER );
            timer4.setAutoReload( 0xFFFF );
            timer4.setAutoReloadPreload( true );

            timer4Channel1.setPolarity( Expander::TIMxCCMR::Polarity::BOTH_EDGES );
            timer4Channel1.setICPrescaler( Expander::TIMxCCMR::ICPrescaler::EVERY_EDGE );
            timer4Channel1.setICFilter( FILTER );
            timer4Channel1.enable();

            timer4.enable();

            Log::info( "Timers on Expander initialized" );
        }

        void _initializeLteTask()
        {
            if ( xTaskCreatePinnedToCore( lteUartTask, "lteUartTask", 4 * 1024, this, LTE_TASK_PRIORITY, NULL, 0 ) !=
                 pdPASS )
            {
                Log::error( "Failed to create lte task" );
            }
        }

        void _initializeTasks()
        {
            if ( xTaskCreatePinnedToCore(
                     componentsTask, "componentsTask", 4 * 1024, this, COMPONENTS_TASK_PRIORITY, NULL, 1
                 ) != pdPASS )
            {
                Log::error( "Failed to create components task" );
            }

            if ( xTaskCreatePinnedToCore( systemTask, "systemTask", 4 * 1024, this, SYSTEM_TASK_PRIORITY, NULL, 1 ) !=
                 pdPASS )
            {
                Log::error( "Failed to create system task" );
            }

            if ( xTaskCreatePinnedToCore(
                     dataSourceTask, "dataSourceTask", 4 * 1024, this, DATA_SOURCE_TASK_PRIORITY, NULL, 1
                 ) != pdPASS )
            {
                Log::error( "Failed to create data source task" );
            }

            if ( xTaskCreatePinnedToCore(
                     controlTask, "controlTask", 8 * 1024, this, CONTROL_TASK_PRIORITY, NULL, 1
                 ) != pdPASS )
            {
                Log::error( "Failed to create control task" );
            }

            if ( xTaskCreatePinnedToCore(
                     communicationTask, "communicationTask", 16 * 1024, this, COMMUNICATION_TASK_PRIORITY, NULL, 0
                 ) != pdPASS )
            {
                Log::error( "Failed to create communication task" );
            }

            if ( xTaskCreatePinnedToCore( streamTask, "streamTask", 8 * 1024, this, STREAM_TASK_PRIORITY, NULL, 0 ) !=
                 pdPASS )
            {
                Log::error( "Failed to create stream task" );
            }

            if ( xTaskCreatePinnedToCore( otaTask, "otaTask", 10 * 1024, this, OTA_TASK_PRIORITY, NULL, 0 ) !=
                 pdPASS )
            {
                Log::error( "Failed to create ota task" );
            }

        }
    };
}    // namespace AsnPlus

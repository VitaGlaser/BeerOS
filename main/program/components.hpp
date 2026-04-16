#pragma once

#include "asn_module_config.hpp"

#include "freertos/FreeRTOS.h"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/timer.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/utils.hpp"

#include "asn/asn-esp32-hal/peripherals/gpio.hpp"
#include "asn/asn-esp32-hal/peripherals/i2c_master.hpp"
#include "asn/asn-esp32-hal/peripherals/uart.hpp"

#include "asn/asn-drivers/pcf85263.hpp"

#include "asn/asn-expander-lib/expander.hpp"
#include "asn/asn-expander-lib/gateway/i2c_gateway.hpp"
#include "asn/asn-expander-lib/hal/gpio.hpp"
#include "asn/asn-expander-lib/spi_transport.hpp"

#include "asn/asn-esp32-ble/nimble.hpp"

#include "asn/asn-esp32-wifi/ethernet/ethernet.hpp"
#include "asn/asn-esp32-wifi/ethernet/w5500_sta.hpp"
#include "asn/asn-esp32-wifi/wifi_manager.hpp"

#include "asn/asn-esp32-modbus/master.hpp"

#include "asn/asn-eg915-driver/eg915.hpp"
#include "asn/asn-eg915-driver/https/https_client.hpp"

#include "components/connection/manager.hpp"

#include "components/cloud/request_manager.hpp"
#include "components/leds/ws2812_led_strip.hpp"

#include "components/measurement/channel_manager.hpp"
#include "components/measurement/data_sources/manager.hpp"
#include "components/measurement/history_manager.hpp"

#include "asn/asn-esp32-hal/time_manager/system_clock.hpp"
#include "components/specific_rtc.hpp"

#include "asn/asn-hal/time_manager/time_manager.hpp"

#include "database/database.hpp"

#include "pinout.hpp"

namespace AsnPlus
{
    class Components
    {
    public:
        Esp32::Nvs nvs {};

        Database database {};

        // MARK: Time related stuff
        Esp32::I2cMaster::Config i2cConfig { 100'000, I2C_NUM_0, Pinout::I2C_SDA, Pinout::I2C_SCL, true };
        Esp32::I2cMaster         i2cMaster { i2cConfig };
        Drivers::PCF85263A       pcfRtc { i2cMaster };

        Esp32::SystemClock systemClock {};
        SpecificRtc        rtc { pcfRtc };

        TimeManager
            timeManager { systemClock, rtc, database.timeConfig, database.timeRuntime, database.timeChangeRequest };

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
        Network::W5500Ethernet    ethernet { database.ethConfig, database.ethRuntime, ethSta };
        Esp32::Https::Client      ethernetClient {};

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
            requestManager
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

        // Esp32::DigitalOutput expanderBoot { Pinout::EXPANDER_BOOT, false, false };
        Expander::SpiTransport transport { spi_host_device_t::SPI3_HOST, Pinout::EXPANDER_CS, 100'000 };
        Expander::Expander     expander { transport };
        Expander::I2CGateway & gateway { expander.getI2CGateway() };

        Expander::PortPinGpio::Config expanderGpioConfig {
            IGpio::Config::PinMode::OUTPUT,
            IGpio::Config::InterruptType::NONE
        };
        Expander::PortPinGpio ltePower { expanderGpioConfig, expander.getPortB().pin( Pinout::LTE_POWER_KEY ) };

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
        DataSource::Manager dataSourceManager { rtuMaster, pulseChannels };
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

            i2cMaster.initialize();
            timeManager.initialize();

            ledStrip.initialize();
            for ( int i = 0; i < 16; i += 2 ) ledStrip.setColor( i + 1, Color { 77, 0, 255 } );
            // ledStrip.clear_all();
            ledStrip.poll();

            _initializeTasks();

            if ( ProjectConfig::LOG_LEVEL >= 2 )
            {
                vTaskDelete( NULL );
            }

            Log::info( "Initialized" );
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
                    ESP_LOGI( "componentsTask", "Last time: %lu", xTaskGetTickCount() - _lastTime );
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

    private:
        static constexpr const char TAG[]                     = "Components";
        using Log                                             = AsnPlus::Logger< ProjectConfig::LOG_LEVEL, TAG >;

        static constexpr uint32_t COMMUNICATION_TASK_PRIORITY = 3;
        static constexpr uint32_t COMPONENTS_TASK_PRIORITY    = 4;
        static constexpr uint32_t CONTROL_TASK_PRIORITY       = 5;
        static constexpr uint32_t DATA_SOURCE_TASK_PRIORITY   = 4;
        static constexpr uint32_t LTE_TASK_PRIORITY           = 3;
        static constexpr uint32_t SYSTEM_TASK_PRIORITY        = 2;

        static constexpr uint32_t COMMUNICATION_TASK_DELAY_MS = 100;
        static constexpr uint32_t COMPONENTS_TASK_DELAY_MS    = 25;
        static constexpr uint32_t CONTROL_TASK_DELAY_MS       = 10;
        static constexpr uint32_t DATA_SOURCE_TASK_DELAY_MS   = 10;
        static constexpr uint32_t LTE_TASK_DELAY_MS           = 100;
        static constexpr uint32_t SYSTEM_TASK_DELAY_MS        = 1000;

        void _initializeExpander()
        {
            _initExpanderSpiBus();

            expanderNrst.set( false );
            Utils::delay( 100 );
            expanderNrst.set( true );
            Utils::delay( 100 );

            transport.initialize();
            expander.initialize();

            uint8_t  major, minor;
            uint16_t patch;
            expander.getSystem().getVersion( major, minor, patch );
            Log::info( "Expander firmware version: %d.%d.%d", major, minor, patch );

            ltePower.initialize();

            // expander.getPortA().enable();
            expander.getPortB().enable();

            _initializeTimers();
        }

        void _initializeTimers()
        {
            static constexpr uint16_t PRESCALER = 1999;
            timer1.setMode( Expander::TIMxCR::Mode::FREQ );
            timer1.setPrescaler( PRESCALER );
            timer1.setAutoReload( 0xFFFF );
            timer1.setAutoReloadPreload( true );

            timer1Channel1.setPolarity( Expander::TIMxCCMR::Polarity::ACTIVE_HIGH );
            timer1Channel1.setICPrescaler( Expander::TIMxCCMR::ICPrescaler::EVERY_EDGE );
            timer1Channel1.setICFilter( 0 );
            timer1Channel1.enable();

            timer1.enable();

            timer2.setMode( Expander::TIMxCR::Mode::FREQ );
            timer2.setPrescaler( PRESCALER );
            timer2.setAutoReload( 0xFFFF );
            timer2.setAutoReloadPreload( true );

            timer2Channel1.setPolarity( Expander::TIMxCCMR::Polarity::ACTIVE_HIGH );
            timer2Channel1.setICPrescaler( Expander::TIMxCCMR::ICPrescaler::EVERY_EDGE );
            timer2Channel1.setICFilter( 0 );
            timer2Channel1.enable();

            timer2.enable();

            timer3.setMode( Expander::TIMxCR::Mode::FREQ );
            timer3.setPrescaler( PRESCALER );
            timer3.setAutoReload( 0xFFFF );
            timer3.setAutoReloadPreload( true );

            timer3Channel1.setPolarity( Expander::TIMxCCMR::Polarity::ACTIVE_HIGH );
            timer3Channel1.setICPrescaler( Expander::TIMxCCMR::ICPrescaler::EVERY_EDGE );
            timer3Channel1.setICFilter( 0 );
            timer3Channel1.enable();

            timer3.enable();

            timer4.setMode( Expander::TIMxCR::Mode::FREQ );
            timer4.setPrescaler( PRESCALER );
            timer4.setAutoReload( 0xFFFF );
            timer4.setAutoReloadPreload( true );

            timer4Channel1.setPolarity( Expander::TIMxCCMR::Polarity::ACTIVE_HIGH );
            timer4Channel1.setICPrescaler( Expander::TIMxCCMR::ICPrescaler::EVERY_EDGE );
            timer4Channel1.setICFilter( 0 );
            timer4Channel1.enable();

            timer4.enable();

            Log::info( "Timers on Expander initialized" );
        }

        void _initExpanderSpiBus()
        {
            spi_bus_config_t bus_config = {
                .mosi_io_num     = Pinout::EXPANDER_SPI_MOSI,
                .miso_io_num     = Pinout::EXPANDER_SPI_MISO,
                .sclk_io_num     = Pinout::EXPANDER_SPI_SCK,
                .quadwp_io_num   = -1,
                .quadhd_io_num   = -1,
                .data4_io_num    = -1,
                .data5_io_num    = -1,
                .data6_io_num    = -1,
                .data7_io_num    = -1,
                .max_transfer_sz = 4096,
            };
            spi_bus_initialize( SPI3_HOST, &bus_config, SPI_DMA_CH_AUTO );
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

            if ( xTaskCreatePinnedToCore(
                     systemTask, "systemTask", 4 * 1024, this, SYSTEM_TASK_PRIORITY, NULL, 1
                 ) != pdPASS )
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
        }
    };
}    // namespace AsnPlus

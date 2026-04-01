#pragma once

#include "asn_module_config.hpp"

#include "freertos/FreeRTOS.h"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/timer.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/utils.hpp"

#include "asn/asn-esp32-hal/peripherals/gpio.hpp"
#include "asn/asn-esp32-hal/peripherals/i2c_master.hpp"

#include "asn/asn-drivers/pcf85263.hpp"

#include "asn/asn-expander-lib/expander.hpp"
#include "asn/asn-expander-lib/gateway/i2c_gateway.hpp"
#include "asn/asn-expander-lib/spi_transport.hpp"

#include "asn/asn-esp32-ble/nimble.hpp"

#include "asn/asn-esp32-wifi/wifi_manager.hpp"

#include "components/connection/manager.hpp"

#include "asn/asn-esp32-modbus/master.hpp"

#include "components/cloud/request_manager.hpp"
#include "components/leds/ws2812_led_strip.hpp"
#include "components/networking/ethernet.hpp"

#include "components/measurement/channel_manager.hpp"
#include "components/measurement/data_sources/manager.hpp"

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
        Esp32::I2cMaster        i2cMaster { i2cConfig };
        Drivers::PCF85263A      pcfRtc { i2cMaster };

        Esp32::SystemClock      systemClock {};
        SpecificRtc             rtc { pcfRtc };

        TimeManager timeManager {
            systemClock,
            rtc,
            database.timeConfig,
            database.timeRuntime,
            database.timeChangeRequest
        };

        // MARK: Communication
        Bluetooth::Nimble nimble { database.advertisingData, database.bluetoothConfig, database.bluetoothState };

        Wifi::WifiManager    wifiManager { database.wifiConfig };
        Esp32::Https::Client wifiClient {};

        Networking::W5500Ethernet ethernet {
            spi_host_device_t::SPI2_HOST,
            Pinout::ETH_MOSI,
            Pinout::ETH_MISO,
            Pinout::ETH_SCK,
            Pinout::ETH_CS,
            Pinout::ETH_nRST,
            Pinout::ETH_nINT,
            20    // clock speed MHz
        };
        Esp32::Https::Client ethernetClient {};

        Cloud::RequestManager requestManager { database };

        Connection::Manager connectionManager {
            database.connectionModuleConfig,
            database.connectionModuleRuntime,
            database.bluetoothState,
            nimble,
            ethernet,
            ethernetClient,
            database.wifiConfig,
            wifiManager,
            wifiClient,
            requestManager
        };

        // MARK: Peripheral Ports
        Modbus::RtuMaster::Config modbusConfig { 9600, UART_PARITY_EVEN, UART_DATA_8_BITS, UART_STOP_BITS_1, 1000 };
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

        // MARK: Application logic
        DataSource::Manager dataSourceManager { rtuMaster };
        ChannelManager channelManager { dataSourceManager, database };

        void initialize()
        {
            Log::info( "Initializing" );

            nvs.initialize();

            database.initialize();

            connectionManager.initialize();

            _initializeExpander();

            rtuMaster.initialize();

            dataSourceManager.initialize();
            channelManager.initialize();

            i2cMaster.initialize();
            timeManager.initialize();

            ledStrip.initialize();
            for ( int i = 0; i < 16; i += 2 ) ledStrip.setColor( i + 1, Color { 77, 0, 255 } );
            // ledStrip.clear_all();
            ledStrip.poll();

            Log::info( "Initialized" );
        }

        void poll()
        {
            connectionManager.poll();

            timeManager.poll();

            // expander.poll();

            // _pollDigmesa();

            Utils::delay( 10 );
        }

    private:
        static constexpr const char TAG[] = "Components";
        using Log                         = AsnPlus::Logger< ProgramConfig::DEBUG_ENABLED, TAG >;

        void _pollDigmesa()
        {
            uint16_t           regs[ 5 ] = {};
            mb_param_request_t req       = {
                .slave_addr = 1,
                .command    = static_cast< uint8_t >( Modbus::Commands::READ_INPUT_REGISTERS ),
                .reg_start  = 0,
                .reg_size   = 5
            };

            // request() returns esp_err_t cast to bool: ESP_OK(0) = false, error = true
            bool ok = rtuMaster.request( &req, regs );
            if ( ok )
            {
                Log::error( "Digmesa: failed to read input registers" );
                return;
            }

            uint16_t volume       = regs[ 0 ];
            uint16_t flow_rate    = regs[ 1 ];
            uint16_t conductivity = regs[ 2 ];
            float    temperature  = static_cast< float >( regs[ 3 ] ) / 10.0f;
            uint16_t cond_uncomp  = regs[ 4 ];

            Log::info(
                "Digmesa: Volume=%u L, Flow=%u ml/min, Cond=%u uS/cm, Temp=%.1f C, Cond(raw)=%u uS/cm",
                volume,
                flow_rate,
                conductivity,
                temperature,
                cond_uncomp
            );
        }

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
    };
}    // namespace AsnPlus

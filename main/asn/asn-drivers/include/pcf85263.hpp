#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "asn/asn-hal/include/peripherals/i2c_master.hpp"
#include "asn/asn-hal/include/time_manager/rtc.hpp"

namespace AsnPlus::Drivers
{
    /**
     * @brief Driver for the PCF85263A real-time clock over I2C, implementing IRtc.
     */
    class Pcf85263a : public IRtc
    {
    public:
        enum CountdownSrcClock
        {
            TIMER_CLOCK_4096HZ   = 0,
            TIMER_CLOCK_64HZ     = 1,
            TIMER_CLOCK_1HZ      = 2,
            TIMER_CLOCK_1PER60HZ = 3
        };

        /**
         * @brief Construct a PCF85263A driver.
         * @param i2c     Reference to an II2cMaster bus instance.
         * @param address 7-bit I2C device address (default 0x51).
         */
        explicit Pcf85263a( II2cMaster & i2c, uint8_t address = I2C_ADDR );

        /**
         * @brief Initialize the RTC. The bus must already be initialized.
         * @return true on success.
         */
        bool initialize() override;

        void poll() override;

        /**
         * @brief Write UTC time to the RTC registers.
         * @param time UTC time to write. No-op if time is not valid.
         */
        void setUtc( const Time & time ) override;

        /**
         * @brief Read the current UTC time from the RTC registers.
         * @return Current UTC time, or a default-constructed Time on I2C failure.
         */
        Time getUtc() override;

    private:
        static constexpr const char TAG[] = "Drivers::Pcf85263a";
        using Log                         = Logger< ModuleConfig::Drivers::LOG_LEVEL, TAG >;

        static constexpr uint8_t I2C_ADDR = 0x51;

        static constexpr uint8_t RTC_CTRL_1         = 0x00;
        static constexpr uint8_t RTC_CTRL_2         = 0x01;
        static constexpr uint8_t RTC_OFFSET         = 0x02;
        static constexpr uint8_t RTC_RAM_BY         = 0x03;

        static constexpr uint8_t RTC_SECOND_ADDR    = 0x01;
        static constexpr uint8_t RTC_MINUTE_ADDR    = 0x02;
        static constexpr uint8_t RTC_HOUR_ADDR      = 0x03;
        static constexpr uint8_t RTC_DAY_ADDR       = 0x04;
        static constexpr uint8_t RTC_WDAY_ADDR      = 0x05;
        static constexpr uint8_t RTC_MONTH_ADDR     = 0x06;
        static constexpr uint8_t RTC_YEAR_ADDR      = 0x07;

        static constexpr uint8_t RTC_SECOND_ALARM   = 0x08;
        static constexpr uint8_t RTC_MINUTE_ALARM   = 0x09;
        static constexpr uint8_t RTC_HOUR_ALARM     = 0x0a;
        static constexpr uint8_t RTC_DAY_ALARM      = 0x0b;
        static constexpr uint8_t RTC_WDAY_ALARM     = 0x0c;

        static constexpr uint8_t RTC_ALARM          = 0x80;
        static constexpr uint8_t RTC_ALARM_AIE      = 0x80;
        static constexpr uint8_t RTC_ALARM_AF       = 0x40;
        static constexpr uint8_t RTC_CTRL_2_DEFAULT = 0x00;
        static constexpr uint8_t RTC_TIMER_FLAG     = 0x08;

        II2cMaster & _i2c;
        uint8_t      _address;

        static uint8_t _decToBcd( uint8_t val );
        static uint8_t _bcdToDec( uint8_t val );
        bool           _writeRegister( uint8_t reg, uint8_t val );
        bool           _readRegisters( uint8_t reg, uint8_t * dest, size_t len );
    };
}    // namespace AsnPlus::Drivers

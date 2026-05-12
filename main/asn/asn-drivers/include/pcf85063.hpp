#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-hal/include/peripherals/i2c_master.hpp"

#include <cstdlib>
#include <cstring>
#include <ctime>

namespace AsnPlus::Drivers
{
    class Pcf85063
    {
    public:
        enum CountdownSrcClock
        {
            TIMER_CLOCK_4096HZ   = 0,
            TIMER_CLOCK_64HZ     = 1,
            TIMER_CLOCK_1HZ      = 2,
            TIMER_CLOCK_1PER60HZ = 3
        };

        Pcf85063( II2cMaster & i2c, uint8_t address = I2C_ADDR );

        bool     initialize();
        void     setTime( uint8_t hour, uint8_t minute, uint8_t sec );
        void     setDate( uint8_t weekday, uint8_t day, uint8_t month, uint16_t yr );
        void     readTime();
        uint8_t  getSecond();
        uint8_t  getMinute();
        uint8_t  getHour();
        uint8_t  getDay();
        uint8_t  getWeekday();
        uint8_t  getMonth();
        uint16_t getYear();
        uint64_t getTimeInSeconds();
        void     setTime( tm & timeinfo );
        bool     isValid() const;
        void     reset();

    private:
        static constexpr const char TAG[]           = "Drivers::Pcf85063";
        using Log                                   = Logger< ModuleConfig::Drivers::LOG_LEVEL, TAG >;

        static constexpr uint8_t I2C_ADDR           = 0x51;

        static constexpr uint8_t RTC_CTRL_1         = 0x00;
        static constexpr uint8_t RTC_CTRL_2         = 0x01;
        static constexpr uint8_t RTC_OFFSET         = 0x02;
        static constexpr uint8_t RTC_RAM_BY         = 0x03;

        static constexpr uint8_t RTC_SECOND         = 0x04;
        static constexpr uint8_t RTC_MINUTE         = 0x05;
        static constexpr uint8_t RTC_HOUR           = 0x06;
        static constexpr uint8_t RTC_DAY            = 0x07;
        static constexpr uint8_t RTC_WDAY           = 0x08;
        static constexpr uint8_t RTC_MONTH          = 0x09;
        static constexpr uint8_t RTC_YEAR           = 0x0A;

        static constexpr uint8_t RTC_SECOND_ALARM   = 0x0B;
        static constexpr uint8_t RTC_MINUTE_ALARM   = 0x0C;
        static constexpr uint8_t RTC_HOUR_ALARM     = 0x0D;
        static constexpr uint8_t RTC_DAY_ALARM      = 0x0E;
        static constexpr uint8_t RTC_WDAY_ALARM     = 0x0F;

        static constexpr uint8_t RTC_TIMER_VAL      = 0x10;
        static constexpr uint8_t RTC_TIMER_MODE     = 0x11;
        static constexpr uint8_t RTC_TIMER_TCF      = 0x08;
        static constexpr uint8_t RTC_TIMER_TE       = 0x04;
        static constexpr uint8_t RTC_TIMER_TIE      = 0x02;
        static constexpr uint8_t RTC_TIMER_TI_TP    = 0x01;

        static constexpr uint8_t RTC_ALARM          = 0x80;
        static constexpr uint8_t RTC_ALARM_AIE      = 0x80;
        static constexpr uint8_t RTC_ALARM_AF       = 0x40;
        static constexpr uint8_t RTC_CTRL_2_DEFAULT = 0x00;
        static constexpr uint8_t RTC_TIMER_FLAG     = 0x08;

        II2cMaster & _i2c;
        uint8_t      _address;

        uint8_t  _hour    = 0;
        uint8_t  _minute  = 0;
        uint8_t  _second  = 0;
        uint8_t  _day     = 0;
        uint8_t  _weekday = 0;
        uint8_t  _month   = 0;
        uint16_t _year    = 0;

        static uint8_t _decToBcd( uint8_t val );
        static uint8_t _bcdToDec( uint8_t val );
        bool           _writeRegister( uint8_t reg, uint8_t val );
        bool           _readRegisters( uint8_t reg, uint8_t * dest, uint8_t len );
    };
}    // namespace AsnPlus::Drivers

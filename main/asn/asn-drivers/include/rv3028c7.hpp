#pragma once

#include "asn/asn-core/types.hpp"
#include "asn/asn-hal/include/peripherals/i2c_master.hpp"
#include <cstring>

namespace AsnPlus::Drivers
{
    class Rv3028c7
    {
    public:
        // MARK: Types

        enum class Registers : uint8_t
        {
            SECONDS,
            MINUTES,
            HOURS,
            WEEKDAY,
            DATE,
            MONTHS,
            YEARS,
            MINUTES_ALM,
            HOURS_ALM,
            DATE_ALM,
            TIMERVAL_0,
            TIMERVAL_1,
            TIMERSTAT_0,
            TIMERSTAT_1,
            STATUS,
            CTRL1,
            CTRL2,
            GPBITS,
            INT_MASK,
            EVENTCTRL,
            COUNT_TS,
            SECONDS_TS,
            MINUTES_TS,
            HOURS_TS,
            DATE_TS,
            MONTH_TS,
            YEAR_TS,
            UNIX_TIME0,
            UNIX_TIME1,
            UNIX_TIME2,
            UNIX_TIME3,
            USER_RAM1,
            USER_RAM2,
            PASSWORD0,
            PASSWORD1,
            PASSWORD2,
            PASSWORD3,
            EEPROM_ADDR,
            EEPROM_DATA,
            EEPROM_CMD,
            ID,
            EEPROM_CLKOUT_REGISTER,
            RV3028_EE_OFFSET_8_1,
            EEPROM_BACKUP_REGISTER = 0x37,
        };

        enum TimeIndex : uint8_t
        {
            TIME_SECONDS,
            TIME_MINUTES,
            TIME_HOURS,
            TIME_WEEKDAY,
            TIME_DATE,
            TIME_MONTH,
            TIME_YEAR,
        };

        // MARK: Constants

        static constexpr uint8_t TIME_ARRAY_LENGTH = 7;

        static constexpr uint8_t TCR_3K            = 0b00;
        static constexpr uint8_t TCR_5K            = 0b01;
        static constexpr uint8_t TCR_9K            = 0b10;
        static constexpr uint8_t TCR_15K           = 0b11;

        static constexpr uint8_t FD_CLKOUT_32K     = 0b000;
        static constexpr uint8_t FD_CLKOUT_8192    = 0b001;
        static constexpr uint8_t FD_CLKOUT_1024    = 0b010;
        static constexpr uint8_t FD_CLKOUT_64      = 0b011;
        static constexpr uint8_t FD_CLKOUT_32      = 0b100;
        static constexpr uint8_t FD_CLKOUT_1       = 0b101;
        static constexpr uint8_t FD_CLKOUT_TIMER   = 0b110;
        static constexpr uint8_t FD_CLKOUT_LOW     = 0b111;

        // MARK: Constructor

        Rv3028c7( II2cMaster & i2c, uint8_t address );

        // MARK: Initialize

        bool initialize(
            bool set24HourMode     = true,
            bool disableTrickle    = true,
            bool setLevelSwitching = true,
            bool clearStatusReg    = true
        );

        // MARK: Time

        bool setTime(
            uint8_t  sec,
            uint8_t  min,
            uint8_t  hour,
            uint8_t  weekday,
            uint8_t  date,
            uint8_t  month,
            uint16_t year
        );
        bool     setTime( uint8_t * time, uint8_t len );
        bool     setSeconds( uint8_t value );
        bool     setMinutes( uint8_t value );
        bool     setHours( uint8_t value );
        bool     setWeekday( uint8_t value );
        bool     setDate( uint8_t value );
        bool     setMonth( uint8_t value );
        bool     setYear( uint16_t value );
        bool     setToCompilerTime();
        bool     updateTime();
        char *   stringDateUSA();
        char *   stringDate();
        char *   stringTime();
        char *   stringTimeStamp();
        uint8_t  getSeconds();
        uint8_t  getMinutes();
        uint8_t  getHours();
        uint8_t  getWeekday();
        uint8_t  getDate();
        uint8_t  getMonth();
        uint16_t getYear();
        bool     is12Hour();
        bool     isPM();
        void     set12Hour();
        void     set24Hour();
        bool     setUNIX( uint32_t value );
        uint32_t getUNIX();

        // MARK: Alarm

        void enableAlarmInterrupt(
            uint8_t min,
            uint8_t hour,
            uint8_t dateOrWeekday,
            bool    weekdayAlarm,
            uint8_t mode,
            bool    enableClockOutput
        );
        void enableAlarmInterrupt();
        void disableAlarmInterrupt();
        bool readAlarmInterruptFlag();
        void clearAlarmInterruptFlag();

        // MARK: Timer

        void setTimer(
            bool     timerRepeat,
            uint16_t timerFrequency,
            uint16_t timerValue,
            bool     setInterrupt,
            bool     startTimer,
            bool     enableClockOutput
        );
        void enableTimerInterrupt();
        void disableTimerInterrupt();
        bool readTimerInterruptFlag();
        void clearTimerInterruptFlag();
        void enableTimer();
        void disableTimer();

        // MARK: Periodic Update Interrupt

        void enablePeriodicUpdateInterrupt( bool everySecond, bool enableClockOutput );
        void disablePeriodicUpdateInterrupt();
        bool readPeriodicUpdateInterruptFlag();
        void clearPeriodicUpdateInterruptFlag();

        // MARK: Trickle Charge

        void enableTrickleCharge( uint8_t tcr );
        void disableTrickleCharge();
        bool setBackupSwitchoverMode( uint8_t val );

        // MARK: Clock Output

        void enableClockOut( uint8_t freq );
        void enableInterruptControlledClockout( uint8_t freq );
        void disableClockOut();
        bool readClockOutputInterruptFlag();
        void clearClockOutputInterruptFlag();

        // MARK: Misc

        uint8_t status();
        void    clearInterrupts();
        void    reset();

        // MARK: User EEPROM

        bool    writeUserEeprom( uint8_t eepromAddr, uint8_t val );
        uint8_t readUserEeprom( uint8_t eepromAddr );

    private:
        // MARK: Register bit positions

        static constexpr uint8_t STATUS_EEBUSY            = 7;
        static constexpr uint8_t STATUS_CLKF              = 6;
        static constexpr uint8_t STATUS_BSF               = 5;
        static constexpr uint8_t STATUS_UF                = 4;
        static constexpr uint8_t STATUS_TF                = 3;
        static constexpr uint8_t STATUS_AF                = 2;
        static constexpr uint8_t STATUS_EVF               = 1;
        static constexpr uint8_t STATUS_PORF              = 0;

        static constexpr uint8_t CTRL1_TRPT               = 7;
        static constexpr uint8_t CTRL1_WADA               = 5;
        static constexpr uint8_t CTRL1_USEL               = 4;
        static constexpr uint8_t CTRL1_EERD               = 3;
        static constexpr uint8_t CTRL1_TE                 = 2;
        static constexpr uint8_t CTRL1_TD1                = 1;
        static constexpr uint8_t CTRL1_TD0                = 0;

        static constexpr uint8_t CTRL2_TSE                = 7;
        static constexpr uint8_t CTRL2_CLKIE              = 6;
        static constexpr uint8_t CTRL2_UIE                = 5;
        static constexpr uint8_t CTRL2_TIE                = 4;
        static constexpr uint8_t CTRL2_AIE                = 3;
        static constexpr uint8_t CTRL2_EIE                = 2;
        static constexpr uint8_t CTRL2_12_24              = 1;
        static constexpr uint8_t CTRL2_RESET              = 0;

        static constexpr uint8_t HOURS_AM_PM              = 5;
        static constexpr uint8_t MINUTESALM_AE_M          = 7;
        static constexpr uint8_t HOURSALM_AE_H            = 7;
        static constexpr uint8_t DATE_AE_WD               = 7;

        static constexpr uint8_t IMT_MASK_CEIE            = 3;
        static constexpr uint8_t IMT_MASK_CAIE            = 2;
        static constexpr uint8_t IMT_MASK_CTIE            = 1;
        static constexpr uint8_t IMT_MASK_CUIE            = 0;

        // MARK: EEPROM constants

        static constexpr uint8_t EEPROMCMD_FIRST          = 0x00;
        static constexpr uint8_t EEPROMCMD_UPDATE         = 0x11;
        static constexpr uint8_t EEPROMCMD_REFRESH        = 0x12;
        static constexpr uint8_t EEPROMCMD_WRITE_SINGLE   = 0x21;
        static constexpr uint8_t EEPROMCMD_READ_SINGLE    = 0x22;

        static constexpr uint8_t EEPROM_BACKUP_TCE_BIT    = 5;
        static constexpr uint8_t EEPROM_BACKUP_FEDE_BIT   = 4;
        static constexpr uint8_t EEPROM_BACKUP_BSM_SHIFT  = 2;
        static constexpr uint8_t EEPROM_BACKUP_TCR_SHIFT  = 0;
        static constexpr uint8_t EEPROM_BACKUP_BSM_CLEAR  = 0b11110011;
        static constexpr uint8_t EEPROM_BACKUP_TCR_CLEAR  = 0b11111100;

        static constexpr uint8_t EEPROM_CLKOUT_CLKOE_BIT  = 7;
        static constexpr uint8_t EEPROM_CLKOUT_CLKSY_BIT  = 6;
        static constexpr uint8_t EEPROM_CLKOUT_PORIE      = 3;
        static constexpr uint8_t EEPROM_CLKOUT_FREQ_SHIFT = 0;

        // MARK: Build-time constants

        static constexpr uint8_t BUILD_MONTH = static_cast< uint8_t >(
            ( ( __DATE__[ 0 ] == 'J' ) && ( __DATE__[ 1 ] == 'a' ) )                                   ? 1
                : ( __DATE__[ 0 ] == 'F' )                                                             ? 2
                : ( ( __DATE__[ 0 ] == 'M' ) && ( __DATE__[ 1 ] == 'a' ) && ( __DATE__[ 2 ] == 'r' ) ) ? 3
                : ( ( __DATE__[ 0 ] == 'A' ) && ( __DATE__[ 1 ] == 'p' ) )                             ? 4
                : ( ( __DATE__[ 0 ] == 'M' ) && ( __DATE__[ 1 ] == 'a' ) && ( __DATE__[ 2 ] == 'y' ) ) ? 5
                : ( ( __DATE__[ 0 ] == 'J' ) && ( __DATE__[ 1 ] == 'u' ) && ( __DATE__[ 2 ] == 'n' ) ) ? 6
                : ( ( __DATE__[ 0 ] == 'J' ) && ( __DATE__[ 1 ] == 'u' ) && ( __DATE__[ 2 ] == 'l' ) ) ? 7
                : ( ( __DATE__[ 0 ] == 'A' ) && ( __DATE__[ 1 ] == 'u' ) )                             ? 8
                : ( __DATE__[ 0 ] == 'S' )                                                             ? 9
                : ( __DATE__[ 0 ] == 'O' )                                                             ? 10
                : ( __DATE__[ 0 ] == 'N' )                                                             ? 11
                                                                                                       : 12
        );

        static constexpr uint8_t BUILD_DATE = static_cast< uint8_t >(
            ( ( __DATE__[ 4 ] == ' ' ) ? 0 : ( __DATE__[ 4 ] - 0x30 ) ) * 10 + ( __DATE__[ 5 ] - 0x30 )
        );

        static constexpr uint16_t BUILD_YEAR = static_cast< uint16_t >(
            ( __DATE__[ 7 ] - 0x30 ) * 1000 + ( __DATE__[ 8 ] - 0x30 ) * 100 + ( __DATE__[ 9 ] - 0x30 ) * 10 +
            ( __DATE__[ 10 ] - 0x30 )
        );

        static constexpr uint8_t BUILD_HOUR = static_cast< uint8_t >(
            ( ( __TIME__[ 0 ] == ' ' ) ? 0 : ( __TIME__[ 0 ] - 0x30 ) ) * 10 + ( __TIME__[ 1 ] - 0x30 )
        );

        static constexpr uint8_t BUILD_MINUTE = static_cast< uint8_t >(
            ( ( __TIME__[ 3 ] == ' ' ) ? 0 : ( __TIME__[ 3 ] - 0x30 ) ) * 10 + ( __TIME__[ 4 ] - 0x30 )
        );

        static constexpr uint8_t BUILD_SECOND = static_cast< uint8_t >(
            ( ( __TIME__[ 6 ] == ' ' ) ? 0 : ( __TIME__[ 6 ] - 0x30 ) ) * 10 + ( __TIME__[ 7 ] - 0x30 )
        );

        // MARK: Data

        uint8_t      _time[ TIME_ARRAY_LENGTH ] = {};
        II2cMaster & _i2c;
        uint8_t      _address;

        // MARK: BCD

        static uint8_t _bcdToDec( uint8_t val );
        static uint8_t _decToBcd( uint8_t val );

        // MARK: I2C helpers

        uint8_t _readRegister( uint8_t addr );
        bool    _writeRegister( uint8_t addr, uint8_t val );
        bool    _readMultipleRegisters( uint8_t addr, uint8_t * dest, uint8_t len );
        bool    _writeMultipleRegisters( uint8_t addr, uint8_t * values, uint8_t len );

        // MARK: EEPROM helpers

        bool    _writeConfigEepromRam( uint8_t eepromAddr, uint8_t val );
        uint8_t _readConfigEepromRam( uint8_t eepromAddr );
        bool    _waitForEeprom();

        // MARK: Bit helpers

        void _setBit( uint8_t regAddr, uint8_t bitNum );
        void _clearBit( uint8_t regAddr, uint8_t bitNum );
        bool _readBit( uint8_t regAddr, uint8_t bitNum );
    };
}    // namespace AsnPlus::Drivers

#pragma once

#include "asn/asn-core/algs.hpp"
#include "asn/asn-core/types.hpp"

#include "asn/asn-hal/include/peripherals/i2c_master.hpp"

namespace AsnPlus::Drivers
{
    /*
        Real Time Clock by Diodes Inc.
        https://www.diodes.com/part/view/PT7C4563BQ?BackID=1197
    */
    class Pt7c4563
    {
    public:
        enum class Status
        {
            OK,
            BusFailure,
            OscillatorFailure,
        };

        enum class Register : uint8_t
        {
            Control1     = 0,
            Control2     = 1,
            Seconds      = 2,
            Minutes      = 3,
            Hours        = 4,
            Days         = 5,
            WeekDay      = 6,
            Months       = 7,
            Years        = 8,
            AlarmMinutes = 9,
            AlarmHours   = 10,
            AlarmDays    = 11,
            AlarmWeekday = 12,
            SquareWave   = 13,
        };

        Pt7c4563( II2cMaster & i2c, uint8_t address );

        void     initialize();
        DateTime readTime();
        Status   writeTime( DateTime dt );

    private:
        II2cMaster & _i2c;
        uint8_t      _address;
    };
}    // namespace AsnPlus::Drivers

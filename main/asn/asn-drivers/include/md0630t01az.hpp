#pragma once

#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-esp32-modbus/include/master.hpp"
#include "asn/asn-hal/include/peripherals/gpio.hpp"

namespace AsnPlus::Drivers
{
    /*
        Driver for MD0630T01A-Z RCD
        Residual Current Detection Leakage Current Sensor
        https://www.ivy-metering.com/uart/148.html
    */
    class Md0630t01az
    {
    public:
        static constexpr uint16_t AC_CURRENT_REG   = 0x0001;
        static constexpr uint16_t DC_CURRENT_REG   = 0x0000;
        static constexpr uint16_t AC_THRESHOLD_REG = 0x0003;
        static constexpr uint16_t DC_THRESHOLD_REG = 0x0002;

        enum LeakState : uint8_t
        {
            STATE_OK_LEAK,
            STATE_DC_LEAK,
            STATE_AC_LEAK,
            STATE_ACDC_LEAK,
            INIT_STATE_LEAK
        };

        using LeakDetectedCallback = Delegate< void( LeakState ) >;

        Md0630t01az(
            Modbus::RtuMaster &  modbusRTU,
            uint8_t              addr,
            IGpio &              dcLeak,
            IGpio &              acLeak,
            IGpio &              uartSw,
            bool                 uartSwActiveHigh,
            LeakDetectedCallback leakDetectedCallback
        );

        LeakState getState();
        void      initialize();
        LeakState checkLeaks();
        bool      setAcThreshold( uint16_t acLeak );
        bool      setDcThreshold( uint16_t dcLeak );
        bool      getAcThreshold( uint16_t & acLeak );
        bool      getDcThreshold( uint16_t & dcLeak );
        float     readAcCurrent();
        float     readDcCurrent();

        static void dcLeakIsrHandler( void * arg );
        static void acLeakIsrHandler( void * arg );

    private:
        Modbus::RtuMaster & _modbusRTU;
        uint8_t             _addr;

        IGpio & _dcLeak;
        IGpio & _acLeak;
        IGpio & _uartSw;

        volatile LeakState _state = LeakState::STATE_OK_LEAK;

        bool                 _uartActiveHigh;
        LeakDetectedCallback _leakDetectedCallback;

        bool  _checkLeak( IGpio & leak );
        bool  _setThreshold( uint16_t threshold, uint16_t regStart );
        float _readAcCurrent();
        float _readDcCurrent();
        bool  _modbusRequest( Modbus::Commands command, uint16_t regStart, uint16_t & data );
    };
}    // namespace AsnPlus::Drivers

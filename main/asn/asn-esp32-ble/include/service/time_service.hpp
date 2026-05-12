#pragma once

#include "../attribute/array_attribute.hpp"
#include "../attribute/command_attribute.hpp"
#include "../attribute/simple_attribute.hpp"
#include "../uuid.hpp"
#include "service.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/timer.hpp"
#include "asn/asn-hal/include/time_manager/structs.hpp"

namespace AsnPlus::Bluetooth
{
    class TimeService
    {
    public:
        TimeService( TimeConfig & config, TimeRuntime & runtime, TimeChangeRequest & request );

        bool initialize();
        void poll();

    private:
        static constexpr const char TAG[]        = "TimeService";
        using Log                                = Logger< ModuleConfig::Ble::LOG_LEVEL, TAG >;

        static constexpr uint32_t TIMER_INTERVAL = 1000;

        static constexpr ble_uuid128_t SERVICE_UUID             = "1de02d1d-2b24-4f4b-a0a0-13e3283e39d9"_uuid;
        static constexpr ble_uuid128_t TIME_CONFIG_UUID         = "4844fcb8-a2e0-4d7b-a0b8-439970537356"_uuid;
        static constexpr ble_uuid128_t TIME_RUNTIME_UUID        = "93774726-e84f-404f-9389-e915bf5533ab"_uuid;
        static constexpr ble_uuid128_t TIME_CHANGE_REQUEST_UUID = "841979b5-fa62-4eb4-b8e5-c1ddaf419b9e"_uuid;

        TimeConfig &        _config;
        TimeRuntime &       _runtime;
        TimeChangeRequest & _changeRequest;

        Timer<> _notifyTimer {};

        Service< 3 > _service;

        SimpleAttribute< TimeConfig >        _timeConfigAttribute;
        SimpleAttribute< TimeRuntime >       _timeRuntimeAttribute;
        SimpleAttribute< TimeChangeRequest > _timeChangeRequestAttribute;
    };
}    // namespace AsnPlus::Bluetooth

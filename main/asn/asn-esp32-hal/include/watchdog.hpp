#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "asn/asn-hal/include/watchdog.hpp"

#include "esp_err.h"
#include "esp_idf_version.h"
#include "esp_task_wdt.h"

namespace AsnPlus::Esp32
{
    class Watchdog : public AsnPlus::IWatchdog
    {
    public:
        Watchdog( Config & cfg );

        bool initialize() override;
        void feed() override;
        void enable() override;
        void disable() override;

    private:
        static constexpr const char TAG[] = "Watchdog";
        using Log                         = Logger< ModuleConfig::Esp32::LOG_LEVEL, TAG >;

        bool _enabled                     = false;
    };
}    // namespace AsnPlus::Esp32

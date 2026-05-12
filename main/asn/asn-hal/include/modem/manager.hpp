#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "modem.hpp"

namespace AsnPlus
{
    class ModemManager
    {
    public:
        using MillisDelegate = Delegate< uint32_t( void ) >;

        explicit ModemManager(
            IModem &       modem,
            MillisDelegate millisDelegate,
            uint32_t       reconnectAttemptIntervalMs = 5000
        );

        bool           initialize();
        void           poll();
        IModem::Status getStatus();

    private:
        static constexpr const char TAG[] = "ModemManager";
        using Log                         = Logger< ModuleConfig::Modem::LOG_LEVEL, TAG >;

        IModem &       _modem;
        MillisDelegate _millisDelegate;
        uint32_t       _reconnectAttemptIntervalMs;
        uint32_t       _lastAttachAttempt;
    };
}    // namespace AsnPlus

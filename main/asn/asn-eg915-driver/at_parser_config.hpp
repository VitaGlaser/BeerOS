#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/types.hpp"

namespace AsnPlus
{
    struct AtParserConfig
    {
        static constexpr size_t LOG_LEVEL = ModuleConfig::Eg915::At::LOG_LEVEL;
        static constexpr size_t URC_PROCESSOR_MAX_HANDLERS_COUNT =
            ModuleConfig::Eg915::At::URC_PROCESSOR_MAX_HANDLERS_COUNT;
        static constexpr size_t URC_PROCESSOR_MAX_PREFIX_SIZE = ModuleConfig::Eg915::At::URC_PROCESSOR_MAX_PREFIX_SIZE;
        static constexpr size_t AT_UART_RX_BUFFER_SIZE        = ModuleConfig::Eg915::At::AT_UART_RX_BUFFER_SIZE;
        static constexpr size_t AT_UART_TX_BUFFER_SIZE        = ModuleConfig::Eg915::At::AT_UART_TX_BUFFER_SIZE;
        static constexpr size_t AT_UART_READ_CHUNK_SIZE       = ModuleConfig::Eg915::At::AT_UART_READ_CHUNK_SIZE;
    };
}    // namespace AsnPlus

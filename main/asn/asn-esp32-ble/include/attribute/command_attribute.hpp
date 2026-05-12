#pragma once

#include "base_attribute.hpp"

namespace AsnPlus::Bluetooth
{
    class CommandAttribute : public BaseAttribute
    {
    public:
        using CALLBACK_TYPE = std::function< void( u8, void * ) >;
        CALLBACK_TYPE command_callback;

        CommandAttribute( ServiceBase & service, ble_uuid128_t uuid, CALLBACK_TYPE command_callback = NULL );

        u8 on_write( ble_gatt_access_ctxt * ctxt ) override;

    private:
        static constexpr const char TAG[] = "CommandAttribute";
        using Log                         = Logger< ModuleConfig::Ble::LOG_LEVEL, TAG >;
    };
}    // namespace AsnPlus::Bluetooth

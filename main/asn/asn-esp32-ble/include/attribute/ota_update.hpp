#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/logger.hpp"

#include "esp_ota_ops.h"
#include "sdkconfig.h"

namespace AsnPlus::Bluetooth
{
    class OtaUpdate
    {
    public:
        using OnUpdateRequestDelegate = Delegate< void() >;

        enum class OtaState : u8
        {
            Idle,
            Requested,
            InProgress,
            Done,
            Failed
        };

        OtaState ota_state_info = OtaState::Idle;

        u8  data_val[ 512 ];
        u8  control_val;
        u16 control_val_handle;

        u8 ota0_fw_ver = 2;
        u8 ota1_fw_ver = 1;

        void registerOnUpdateRequest( OnUpdateRequestDelegate * cb );

        void data_cb();
        void control( u16 conn_handle );
        bool gap_connected();
        void choose_partition();
        void check();

    protected:
        OnUpdateRequestDelegate * _onUpdateRequestCb = nullptr;

        bool _run_diagnostics();

        typedef enum
        {
            SVR_CONTROL_NOP,
            SVR_CONTROL_REQUEST,
            SVR_CONTROL_REQUEST_ACK,
            SVR_CONTROL_REQUEST_NAK,
            SVR_CONTROL_DONE,
            SVR_CONTROL_DONE_ACK,
            SVR_CONTROL_DONE_NAK,
        } svr_chr_ota_control_val_t;

        const esp_partition_t * _update_partition = nullptr;
        esp_ota_handle_t        _update_handle    = 0;

        u16       _packet_size               = 0;
        u16       _num_pkgs_received         = 0;
        bool      _updating                  = false;
        const u16 _reboot_deep_sleep_timeout = 500;

    private:
        static constexpr const char TAG[] = "OtaUpdateAttribute";
        using Log                         = Logger< ModuleConfig::Ble::LOG_LEVEL, TAG >;
    };
}    // namespace AsnPlus::Bluetooth

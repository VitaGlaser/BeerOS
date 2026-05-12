#pragma once

#include "host/ble_gap.h"

#include "asn/asn-core/span.hpp"

namespace AsnPlus::Bluetooth
{
    class AdvData
    {
    public:
        virtual IBytes get_name();
        virtual IBytes get_mfg_data();

        ble_hs_adv_fields fields();

    private:
        uint8_t _mfg_data[ 32 ];
    };
}    // namespace AsnPlus::Bluetooth

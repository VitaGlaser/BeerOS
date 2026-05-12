#pragma once

#include "asn/asn-core/types.hpp"
#include "host/ble_gatt.h"

namespace AsnPlus::Bluetooth
{
    enum class Properties : u16
    {
        ReadOnly        = BLE_GATT_CHR_F_READ,
        WriteOnly       = BLE_GATT_CHR_F_WRITE,
        WriteNotify     = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_NOTIFY,
        ReadWrite       = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        ReadNotify      = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
        ReadWriteNotify = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_NOTIFY
    };
}    // namespace AsnPlus::Bluetooth

#pragma once

#include "asn/asn-esp32-ble/include/adv_data.hpp"

namespace AsnPlus
{
    class AdvertisingData : public AsnPlus::Bluetooth::AdvData
    {
    public:
        const char *        device_name           = "BeerOS";
        static constexpr u8 SERIAL_NUMBER_MAX_LEN = 16;

        struct ManufacturerSpecific
        {
            char serial_number[ SERIAL_NUMBER_MAX_LEN ] =
                { 'A', 'S', 'N', 'P', 'D', 'T', 'T', 'Y', 'P', 'R', '0', '0', '0', '0', '0', '0' };
        } manufacturer_specific;

        IBytes get_name() override { return { (u8 *) device_name, strlen( device_name ) }; }

        IBytes get_mfg_data() override { return { (u8 *) &manufacturer_specific, sizeof( manufacturer_specific ) }; }
    };
}    // namespace AsnPlus

#pragma once

#include "../attribute/ota_attribute.hpp"
#include "../attribute/ota_update.hpp"
#include "../uuid.hpp"
#include "service.hpp"

namespace AsnPlus::Bluetooth
{
    class OtaService : public Service< 2 >
    {
    public:
        OtaUpdate           update;
        OtaControlAttribute control;
        OtaDataAttribute    data;

        OtaService();

    private:
        static constexpr ble_uuid128_t SERVICE_UUID = "d6f1d96d-594c-4c53-b1c6-244a1dfde6d8"_uuid;
        static constexpr ble_uuid128_t CONTROL_UUID = "7ad671aa-21c0-46a4-b722-270e3ae3d831"_uuid;
        static constexpr ble_uuid128_t DATA_UUID    = "23408888-1f40-4cd8-9b89-ca8d45f8a5b1"_uuid;
    };
}    // namespace AsnPlus::Bluetooth

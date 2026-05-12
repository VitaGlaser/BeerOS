#pragma once

#include "../attribute/simple_attribute.hpp"
#include "../uuid.hpp"
#include "service.hpp"

#include "asn/asn-core/logger.hpp"

#include "asn/asn-hal/include/common/identification_structs.hpp"

#include "asn/asn-hal/include/peripherals/persistent_storage.hpp"

namespace AsnPlus::Bluetooth
{
    class IdentificationService
    {
    public:
        IdentificationService( IPersistentStorage & storage, FirmwareInfo & fwInfo, ManufactureInfo & manufacturerInfo, OwnerInfo & ownerInfo );

        bool initialize();

    private:
        static constexpr const char TAG[] = "IdentificationService";
        using Log                         = Logger< ModuleConfig::Ble::LOG_LEVEL, TAG >;

        static constexpr ble_uuid128_t SERVICE_UUID           = "4ca5712a-f319-4bfb-8211-1e2337ecaf8f"_uuid;
        static constexpr ble_uuid128_t FIRMWARE_INFO_UUID     = "e6e4a209-307f-479c-8ff5-e3c3d7c0cc95"_uuid;
        static constexpr ble_uuid128_t MANUFACTURER_INFO_UUID = "fd174313-4c5a-42be-b3fb-24e47024982f"_uuid;
        static constexpr ble_uuid128_t OWNER_INFO_UUID        = "590a84f2-6e00-42b2-a879-7c4bb10b0ea2"_uuid;

        static constexpr const char MFG_INFO_NVS_KEY[] = "mfg_info";
        static constexpr const char OWNER_INFO_NVS_KEY[] = "owner_info";

        IPersistentStorage & _storage;

        FirmwareInfo &    _firmwareInfo;
        ManufactureInfo & _manufacturerInfo;
        OwnerInfo &       _ownerInfo;

        Service< 3 > _service;

        SimpleAttribute< FirmwareInfo >    _firmwareInfoAttribute;
        SimpleAttribute< ManufactureInfo > _manufacturerInfoAttribute;
        SimpleAttribute< OwnerInfo >       _ownerInfoAttribute;

        bool _storeManufactureInfo();
        bool _loadManufactureInfo();
        bool _storeOwnerInfo();
        bool _loadOwnerInfo();
    };
}    // namespace AsnPlus::Bluetooth

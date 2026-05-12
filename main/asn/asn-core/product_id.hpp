#pragma once

#include "string.hpp"
#include "types.hpp"

namespace AsnPlus
{
    class ProductId
    {
    public:
        static constexpr uint8_t PRODUCT_ID_PART_COUNT      = 5;
        static constexpr uint8_t MANUFACTURER_PART_LENGTH   = 3;
        static constexpr uint8_t MANUFACTURER_PART_OFFSET   = 0;

        static constexpr uint8_t PRODUCT_FAMILY_PART_LENGTH = 3;
        static constexpr uint8_t PRODUCT_FAMILY_PART_OFFSET = MANUFACTURER_PART_OFFSET + MANUFACTURER_PART_LENGTH;

        static constexpr uint8_t PRODUCT_TYPE_PART_LENGTH   = 3;
        static constexpr uint8_t PRODUCT_TYPE_PART_OFFSET   = PRODUCT_FAMILY_PART_OFFSET + PRODUCT_FAMILY_PART_LENGTH;

        static constexpr uint8_t HARDWARE_REVISION_PART_LENGTH = 1;
        static constexpr uint8_t HARDWARE_REVISION_PART_OFFSET = PRODUCT_TYPE_PART_OFFSET + PRODUCT_TYPE_PART_LENGTH;

        static constexpr uint8_t SERIAL_NUMBER_PART_LENGTH     = 6;
        static constexpr uint8_t SERIAL_NUMBER_PART_OFFSET =
            HARDWARE_REVISION_PART_OFFSET + HARDWARE_REVISION_PART_LENGTH;

        static constexpr uint8_t PFID_LENGTH      = MANUFACTURER_PART_LENGTH + PRODUCT_FAMILY_PART_LENGTH;
        static constexpr uint8_t PID_LENGTH       = PFID_LENGTH + PRODUCT_TYPE_PART_LENGTH;
        static constexpr uint8_t PN_LENGTH        = PID_LENGTH + HARDWARE_REVISION_PART_LENGTH;
        static constexpr uint8_t UID_LENGTH       = PN_LENGTH + SERIAL_NUMBER_PART_LENGTH;
        static constexpr uint8_t UID_C_STR_LENGTH = UID_LENGTH + 1;

        using PfidString                          = String< PFID_LENGTH >;
        using PidString                           = String< PID_LENGTH >;
        using PnString                            = String< PN_LENGTH >;
        using UidString                           = String< UID_LENGTH >;

        using ManufacturerString                  = String< MANUFACTURER_PART_LENGTH >;
        using ProductFamilyString                 = String< PRODUCT_FAMILY_PART_LENGTH >;
        using ProductTypeString                   = String< PRODUCT_TYPE_PART_LENGTH >;
        using HardwareRevisionString              = String< HARDWARE_REVISION_PART_LENGTH >;
        using SerialNumberString                  = String< SERIAL_NUMBER_PART_LENGTH >;

        using FormattedProductIdString            = String< UID_LENGTH + PRODUCT_ID_PART_COUNT - 1 >;

        ProductId( const String< UID_LENGTH > & data ) : _data( data ) {}

        PfidString getPFID() { return _data.substr( 0, PFID_LENGTH ); }

        PidString getPID() { return _data.substr( 0, PID_LENGTH ); }

        PnString getPN() { return _data.substr( 0, PN_LENGTH ); }

        UidString getUID() { return _data; }

        ManufacturerString getManufatacturer()
        {
            return _data.substr( MANUFACTURER_PART_OFFSET, MANUFACTURER_PART_LENGTH );
        }

        ProductFamilyString getProductFamily()
        {
            return _data.substr( PRODUCT_FAMILY_PART_OFFSET, PRODUCT_FAMILY_PART_LENGTH );
        }

        ProductTypeString getProductType()
        {
            return _data.substr( PRODUCT_TYPE_PART_OFFSET, PRODUCT_TYPE_PART_LENGTH );
        }

        HardwareRevisionString getHardwareRevision()
        {
            return _data.substr( HARDWARE_REVISION_PART_OFFSET, HARDWARE_REVISION_PART_LENGTH );
        }

        SerialNumberString getSerialNumber()
        {
            return _data.substr( SERIAL_NUMBER_PART_OFFSET, SERIAL_NUMBER_PART_LENGTH );
        }

        FormattedProductIdString toString()
        {
            FormattedProductIdString result;
            result.append( getManufatacturer() );
            result.append( "-" );
            result.append( getProductFamily() );
            result.append( "-" );
            result.append( getProductType() );
            result.append( "-" );
            result.append( getHardwareRevision() );
            result.append( "-" );
            result.append( getSerialNumber() );
            return result;
        }

    private:
        String< UID_LENGTH > _data;
    };
}    // namespace AsnPlus

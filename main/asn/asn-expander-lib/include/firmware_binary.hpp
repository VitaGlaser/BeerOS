#pragma once

#include <cstddef>
#include <cstdint>

extern const uint8_t _binary_asn_expander_firmware_bin_start[] asm( "_binary_asn_expander_firmware_bin_start" );
extern const uint8_t _binary_asn_expander_firmware_bin_end[] asm( "_binary_asn_expander_firmware_bin_end" );

namespace AsnPlus
{
    namespace Expander
    {
        namespace Firmware
        {

            inline const uint8_t * expander_embedded_firmware = _binary_asn_expander_firmware_bin_start;
            inline const size_t    expander_embedded_firmware_size =
                _binary_asn_expander_firmware_bin_end - _binary_asn_expander_firmware_bin_start;

            inline uint32_t getSignature()
            {
                const uint8_t * fwPtr = expander_embedded_firmware + 0xC0;
                return fwPtr[ 3 ] << 24 | fwPtr[ 2 ] << 16 | fwPtr[ 1 ] << 8 | fwPtr[ 0 ];
            }

        }    // namespace Firmware
    }    // namespace Expander
}    // namespace AsnPlus

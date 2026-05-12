#pragma once

#include "asn/asn-core/span.hpp"
#include "asn/asn-core/types.hpp"

namespace AsnPlus::Usb
{
    class IDevice
    {
    public:
        struct Config
        {
            const char * manufacturerString = "AsnPlus";
            const char * productString      = "HID Device";
            const char * serialNumberString = "00000001";
            uint32_t     keyDownMs          = 8;
            uint32_t     keyUpMs            = 8;
        };

        // NOTE (DK): There should be some defined descriptors in the derived classes
        // Public:
        // static constexpr uint8_t DESCRIPTOR_REPORT[] = { /* ... */ };
        // static constexpr uint16_t DESCRIPTOR_REPORT_LENGTH = sizeof(DESCRIPTOR_REPORT);
        // Private:
        // static constexpr uint16_t REPORT_SIZE = ... ;
        // static constexpr uint8_t POLL_MS = ... ;
        // static constexpr uint16_t DEVICE_DESCRIPTOR_LENGTH = sizeof(tusb_desc_device_t);
        // static constexpr tusb_desc_device_t DEVICE_DESCRIPTOR = { /* ... */ };
        // static constexpr uint16_t STRING_DESCRIPTOR_LENGTH =  /* ... */ ;
        // static constexpr uint8_t STRING_DESCRIPTOR[] = { /* ... */ };
        // static constexpr uint16_t CONFIGURATION_DESCRIPTOR_LENGTH =  TUD_CONFIG_DESC_LEN + ... ;
        // static constexpr uint8_t CONFIGURATION_DESCRIPTOR[] = { /* ... */ };

        IDevice(Config & cfg ) : _config( cfg ) {}

        virtual bool initialize() = 0;
        virtual void poll()       = 0;

    protected:
        Config & _config;

        virtual bool sendReport( IConstBytes report )     = 0;
        virtual bool isMounted() const                    = 0;
        virtual bool pressAndRelease( IConstBytes usage ) = 0;
    };
}    // namespace AsnPlus::Usb

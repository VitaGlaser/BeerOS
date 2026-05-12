#pragma once

#include "asn/asn-core/string.hpp"

namespace AsnPlus
{
    class IModem
    {
    public:
        enum class Status
        {
            ERROR,
            UNINITIALIZED,
            NO_SIM,
            DISCONNECTED,
            CONNECTING,
            CONNECTED,
        };

        virtual Status getStatus() const                                                      = 0;

        virtual bool initialize()                                                             = 0;

        virtual bool configureApn( StringView apn, StringView username, StringView password ) = 0;

        virtual bool attach()                                                                 = 0;

        // Can be potentially used for designs without a HW reset pin
        virtual bool softwareReset() { return false; }
    };
}    // namespace AsnPlus

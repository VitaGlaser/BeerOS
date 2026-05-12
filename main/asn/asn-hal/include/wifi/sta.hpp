#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/array.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "structs.hpp"

namespace AsnPlus::Wifi
{
    class ISta
    {
    public:
        static constexpr uint8_t MAX_SCANNED_NETWORKS = 32;
        enum class State : uint8_t
        {
            UNKNOWN,
            SCANNING,
            SCAN_DONE,
            CONNECTING,
            CONNECTED,
            DISCONNECTING,
            DISCONNECTED,
            ERROR
        };

        struct Runtime
        {
            State      state = State::UNKNOWN;
            WifiConfig currentConnectedWifi;
        };

        using ScannedNetworks = Array< WifiConfig, MAX_SCANNED_NETWORKS >;

        ISta( Runtime & runtime, ScannedNetworks & scannedNetworks ) :
            _runtime( runtime ),
            _scannedNetworks( scannedNetworks )
        {
        }

        virtual bool initialize()                   = 0;
        virtual void poll()                         = 0;

        virtual void startScan()                    = 0;
        virtual void stopScan()                     = 0;

        virtual void connect( WifiConfig & config ) = 0;
        virtual void disconnect()                   = 0;

    protected:
        Runtime &         _runtime;
        ScannedNetworks & _scannedNetworks;
    };
}    // namespace AsnPlus::Wifi
#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "sta.hpp"
#include "structs.hpp"

namespace AsnPlus::Wifi
{
    class Manager
    {
    public:
        enum class Command : uint8_t
        {
            NO_COMMAND,
            CONFIG_START,
            CONFIG_END,
            SCAN,
            CONNECT,
            DISCONNECT
        };

        struct Config
        {
            bool enabled     = true;
            bool autoConnect = true;
        };

        struct Runtime
        {
            bool configMode = false;
        };

        struct Request
        {
            static constexpr uint8_t COMMAND_DATA_LENGTH = 128;
            Command                  command             = Command::NO_COMMAND;
            uint8_t                  reserved[ 3 ];
            uint8_t                  commandData[ COMMAND_DATA_LENGTH ];
        };

        using SavedNetworks   = Array< Wifi::WifiConfig, ModuleConfig::Wifi::MAX_SAVED_NETWORKS >;
        using ScannedNetworks = Wifi::ISta::ScannedNetworks;

        Manager(
            Config &              config,
            Request &             request,
            Runtime &             runtime,
            Wifi::ISta &          sta,
            Wifi::ISta::Runtime & staRuntime,
            ScannedNetworks &     scannedNetworks,
            SavedNetworks &       savedNetworks
        ) :
            _config( config ),
            _request( request ),
            _runtime( runtime ),
            _sta( sta ),
            _staRuntime( staRuntime ),
            _scannedNetworks( scannedNetworks ),
            _savedNetworks( savedNetworks )
        {
        }

        bool initialize();
        void poll();

    private:
        static constexpr const char TAG[] = "Manager";
        using Log                         = Logger< ModuleConfig::Wifi::LOG_LEVEL, TAG >;

        Config &                      _config;
        Request &                     _request;
        Runtime &                     _runtime;
        Wifi::ISta &                  _sta;
        Wifi::ISta::Runtime &         _staRuntime;
        Wifi::ISta::ScannedNetworks & _scannedNetworks;
        SavedNetworks &               _savedNetworks;

        void _processCommand();
        void _autoConnect();
    };
}    // namespace AsnPlus::Wifi

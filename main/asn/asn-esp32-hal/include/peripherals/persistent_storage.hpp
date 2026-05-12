#pragma once

#include "asn_module_config.hpp"

#include "nvs.h"
#include "nvs_flash.h"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "asn/asn-hal/include/peripherals/persistent_storage.hpp"

namespace AsnPlus::Esp32
{
    class PersistentStorage : public IPersistentStorage
    {
    public:
        struct Config : public IPersistentStorage::Config
        {
            const char * partition = "nvs";
        };

        PersistentStorage( Config & config );

        bool initialize() override;
        bool commit() override;
        bool namespaceExists( const char * namespaceTag ) override;
        bool createNamespace( const char * namespaceTag ) override;
        bool deleteNamespace( const char * namespaceTag ) override;
        bool clearNamespace( const char * namespaceTag ) override;
        bool valueExists( const char * key, const char * namespaceTag ) override;
        bool valueExists( const char * key ) override;
        bool loadFromNamespace( const char * key, const char * namespaceTag, uint8_t * data, size_t * len ) override;
        bool storeToNamespace( const char * key, const char * namespaceTag, const uint8_t * data, size_t len ) override;
        bool eraseValue( const char * key, const char * namespaceTag ) override;
        void clear() override;

    private:
        static constexpr const char TAG[] = "PersistentStorage";
        using Log                         = Logger< ModuleConfig::Esp32::LOG_LEVEL, TAG >;

        Config & _config;
    };
}    // namespace AsnPlus::Esp32

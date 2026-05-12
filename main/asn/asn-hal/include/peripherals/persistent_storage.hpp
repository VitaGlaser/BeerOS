#pragma once

#include "asn/asn-core/types.hpp"

namespace AsnPlus
{
    class IPersistentStorage
    {
    public:
        struct Config
        {
            static constexpr uint8_t MAX_KEY_SIZE        = 15;

            char defaultNamespaceTag[ MAX_KEY_SIZE + 1 ] = "namespace";
        };

        IPersistentStorage( Config & config ) : _config( config ) {}

        virtual bool initialize()                                                                                   = 0;

        virtual bool commit()                                                                                       = 0;

        virtual bool namespaceExists( const char * namespaceTag )                                                   = 0;

        virtual bool createNamespace( const char * namespaceTag )                                                   = 0;
        virtual bool deleteNamespace( const char * namespaceTag )                                                   = 0;
        virtual bool clearNamespace( const char * namespaceTag )                                                    = 0;

        virtual bool valueExists( const char * key, const char * namespaceTag )                                     = 0;
        virtual bool valueExists( const char * key )                                                                = 0;

        virtual bool loadFromNamespace( const char * key, const char * namespaceTag, uint8_t * data, size_t * len ) = 0;

        bool load( const char * key, uint8_t * data, size_t * len )
        {
            return loadFromNamespace( key, _config.defaultNamespaceTag, data, len );
        }

        virtual bool
            storeToNamespace( const char * key, const char * namespaceTag, const uint8_t * data, size_t len ) = 0;

        bool store( const char * key, const uint8_t * data, size_t len )
        {
            return storeToNamespace( key, _config.defaultNamespaceTag, data, len );
        }

        virtual bool eraseValue( const char * key, const char * namespaceTag ) = 0;

        bool eraseValue( const char * key ) { return eraseValue( key, _config.defaultNamespaceTag ); }

        virtual void clear() = 0;

    protected:
        Config & _config;

    private:
        static constexpr const char TAG[] = "IPersistentStorage";
    };
}    // namespace AsnPlus

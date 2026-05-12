#ifndef _COMPONENTS_ESP32_NVS_HPP
#define _COMPONENTS_ESP32_NVS_HPP

#include "asn_module_config.hpp"

#include "string.h"

#include "nvs_flash.h"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

// TODO: Rewrite to support multiple partitions, not hardcoded

namespace AsnPlus::Esp32
{
    class Nvs
    {
    public:
        static constexpr const char TAG[]               = "NVS";
        using Log                                       = Logger< ModuleConfig::Esp32::LOG_LEVEL, TAG >;

        static constexpr const char * NAMESPACE         = "namespace";
        static constexpr const char * NAMESPACE_FACTORY = "namespace_2";

        static constexpr const char * PARTITION_1       = "nvs";
        static constexpr const char * PARTITION_FACTORY = "nvs_factory";

        static void initialize()
        {
            Log::debug( "initializing" );
            esp_err_t ret = nvs_flash_init_partition( PARTITION_1 );

            if ( ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND )
            {
                ESP_ERROR_CHECK( nvs_flash_erase_partition( PARTITION_1 ) );
                ret = nvs_flash_init_partition( PARTITION_1 );
            }

            ret = nvs_flash_init_partition( PARTITION_FACTORY );

            if ( ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND )
            {
                ESP_ERROR_CHECK( nvs_flash_erase_partition( PARTITION_FACTORY ) );
                ret = nvs_flash_init_partition( PARTITION_FACTORY );
            }

            nvs_handle_t nvs_handle;

            ret = nvs_open( NAMESPACE, NVS_READWRITE, &nvs_handle );
            if ( ret != ESP_OK )
            {
                Log::error( "error open: %s", esp_err_to_name( ret ) );
            }

            nvs_close( nvs_handle );

            Log::debug( "initializing: %s", esp_err_to_name( ret ) );
            ESP_ERROR_CHECK( ret );
            Log::info( "initialized" );
        }

        static void erase()
        {
            Log::debug( "erasing" );
            ESP_ERROR_CHECK( nvs_flash_erase() );
            initialize();
        }

        template< typename VALUE_TYPE >
        static esp_err_t store_config(
            VALUE_TYPE & value,
            char *       key,
            const char * partition     = PARTITION_1,
            const char * nvs_namespace = NAMESPACE
        )
        {
            Log::debug( "store config size: %u", sizeof( VALUE_TYPE ) );
            return store_config( value, sizeof( VALUE_TYPE ), key, partition, nvs_namespace );
        }

        template< typename VALUE_TYPE >
        static esp_err_t store_config(
            VALUE_TYPE & value,
            size_t       value_size,
            char *       key,
            const char * partition     = PARTITION_1,
            const char * nvs_namespace = NAMESPACE
        )
        {
            nvs_handle_t nvs_handle;
            esp_err_t    ret;

            ret = nvs_open_from_partition( partition, nvs_namespace, NVS_READWRITE, &nvs_handle );
            if ( ret != ESP_OK )
            {
                Log::error( "error open: %s", esp_err_to_name( ret ) );
                return ret;
            }

            ret = nvs_set_blob( nvs_handle, key, &value, value_size );
            if ( ret != ESP_OK )
            {
                Log::error( "error set blob: %s", esp_err_to_name( ret ) );
                return ret;
            }

            ret = nvs_commit( nvs_handle );
            if ( ret != ESP_OK )
            {
                Log::error( "error commit: %s", esp_err_to_name( ret ) );
                return ret;
            }

            nvs_close( nvs_handle );
            Log::debug( "config stored" );
            return ESP_OK;
        }

        static esp_err_t store_config(
            char *       value,
            char *       key,
            const char * partition     = PARTITION_1,
            const char * nvs_namespace = NAMESPACE
        )
        {
            nvs_handle_t nvs_handle;
            esp_err_t    ret;

            ret = nvs_open_from_partition( partition, nvs_namespace, NVS_READWRITE, &nvs_handle );
            if ( ret != ESP_OK )
            {
                Log::error( "error open: %s", esp_err_to_name( ret ) );
                return ret;
            }

            ret = nvs_set_blob( nvs_handle, key, value, strlen( value ) );
            if ( ret != ESP_OK )
            {
                Log::error( "error set blob: %s", esp_err_to_name( ret ) );
                return ret;
            }

            ret = nvs_commit( nvs_handle );
            if ( ret != ESP_OK )
            {
                Log::error( "error commit: %s", esp_err_to_name( ret ) );
                return ret;
            }
            nvs_close( nvs_handle );
            Log::info( "config stored" );
            return ESP_OK;
        }

        template< typename VALUE_TYPE >
        static esp_err_t load_config(
            VALUE_TYPE & value,
            char *       key,
            const char * partition     = PARTITION_1,
            const char * nvs_namespace = NAMESPACE
        )
        {
            return load_config( value, sizeof( VALUE_TYPE ), key, partition, nvs_namespace );
        }

        template< typename VALUE_TYPE >
        static esp_err_t load_config(
            VALUE_TYPE & value,
            size_t       value_size,
            char *       key,
            const char * partition     = PARTITION_1,
            const char * nvs_namespace = NAMESPACE
        )
        {
            nvs_handle_t nvs_handle;
            esp_err_t    ret;

            Log::info( "config load function %s", key );

            ret = nvs_open_from_partition( partition, nvs_namespace, NVS_READONLY, &nvs_handle );
            if ( ret != ESP_OK )
            {
                Log::error( "error open: %s", esp_err_to_name( ret ) );
                return ret;
            }

            ret = nvs_get_blob( nvs_handle, key, &value, &value_size );
            if ( ret != ESP_OK )
            {
                Log::error( "error get blob: %s, %s", esp_err_to_name( ret ), key );
                return ret;
            }

            Log::info( "config loaded" );

            nvs_close( nvs_handle );

            return ESP_OK;
        }

        static esp_err_t load_config(
            char *       value,
            size_t       value_len,
            char *       key,
            const char * partition     = PARTITION_1,
            const char * nvs_namespace = NAMESPACE
        )
        {
            nvs_handle_t nvs_handle;
            esp_err_t    ret;

            Log::info( "config loaded function" );

            ret = nvs_open_from_partition( partition, nvs_namespace, NVS_READONLY, &nvs_handle );
            if ( ret != ESP_OK )
            {
                Log::error( "error open: %s", esp_err_to_name( ret ) );
                return ret;
            }

            ret = nvs_get_blob( nvs_handle, key, &value, &value_len );
            if ( ret != ESP_OK )
            {
                Log::error( "error get blob: %s", esp_err_to_name( ret ) );
                return ret;
            }

            Log::info( "config loaded" );

            nvs_close( nvs_handle );

            return ESP_OK;
        }

        static esp_err_t erase_config(
            const char * key,
            const char * partition     = PARTITION_1,
            const char * nvs_namespace = NAMESPACE
        )
        {
            nvs_handle_t nvs_handle;
            esp_err_t    ret = nvs_open_from_partition( partition, nvs_namespace, NVS_READWRITE, &nvs_handle );
            if ( ret != ESP_OK )
            {
                Log::error( "error open: %s", esp_err_to_name( ret ) );
                return ret;
            }

            ret = nvs_erase_key( nvs_handle, key );
            if ( ret != ESP_OK )
            {
                Log::error( "error erase key: %s", esp_err_to_name( ret ) );
                nvs_close( nvs_handle );
                return ret;
            }

            ret = nvs_commit( nvs_handle );
            if ( ret != ESP_OK )
            {
                Log::error( "error commit: %s", esp_err_to_name( ret ) );
            }

            nvs_close( nvs_handle );
            Log::info( "key erased: %s", key );
            return ret;
        }
    };
}    // namespace AsnPlus::Esp32

#endif

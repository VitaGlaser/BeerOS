#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/logger.hpp"

#include "host/ble_hs.h"

namespace AsnPlus::Bluetooth
{
    static constexpr ble_gatt_svc_def NULL_SERVICE { {}, {}, {}, {} };
    static constexpr ble_gatt_chr_def NULL_CHARACTERISTIC { {}, {}, {}, {}, {}, {}, {}, {} };

    class ServiceBase
    {
    public:
        static constexpr u8 SERVICE_CAPACITY = ModuleConfig::Ble::MAX_SERVICE_COUNT;

        virtual ble_gatt_svc_def get_native()                               = 0;
        virtual void             add( ble_gatt_chr_def new_characteristic ) = 0;

        static ble_gatt_svc_def * service_list();

        void add_self();

    private:
        static constexpr const char TAG[] = "ServiceBase";
        using Log                         = Logger< ModuleConfig::Ble::LOG_LEVEL, TAG >;
    };

    template< u8 CHARACT_CAPACITY >
    class Service : public ServiceBase
    {
    public:
        Service( ble_uuid128_t uuid ) : _uuid { uuid } { ServiceBase::add_self(); }

        void add( ble_gatt_chr_def new_characteristic )
        {
            if ( _count < CHARACT_CAPACITY )
            {
                _characteristics[ _count++ ] = new_characteristic;
                Log::debug( "Added characteristic to service" );
            }
            else
            {
                Log::error( "Cannot add more than %d characteristics to a service", CHARACT_CAPACITY );
            }
        }

        ble_gatt_svc_def get_native()
        {
            _characteristics[ _count ] = NULL_CHARACTERISTIC;

            return ble_gatt_svc_def {
                .type = BLE_GATT_SVC_TYPE_PRIMARY,
                .uuid = &_uuid.u,
                .includes {},
                .characteristics = _characteristics,
            };
        }

    protected:
        ble_uuid128_t    _uuid;
        u8               _count = 0;
        ble_gatt_chr_def _characteristics[ CHARACT_CAPACITY + 1 ] { NULL_CHARACTERISTIC };

        static constexpr const char TAG[] = "Service";
        using Log                         = Logger< ModuleConfig::Ble::LOG_LEVEL, TAG >;
    };
}    // namespace AsnPlus::Bluetooth

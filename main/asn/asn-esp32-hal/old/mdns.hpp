#ifndef _ASNPLUS_ESP32_MDNS_HPP
#define _ASNPLUS_ESP32_MDNS_HPP

#include "asn/asn-core/etl/string.h"
#include <esp_err.h>
#include <mdns.h>
#include <string>

namespace AsnPlus::Esp32
{
    class Mdns
    {
    public:
        Mdns() = default;

        void initialize() { mdns_init(); }

        void setHostname( const char * hostname ) { mdns_hostname_set( hostname ); }

        void setInstanceName( const char * instance_name ) { mdns_instance_name_set( instance_name ); }

        void addService( const char * service, const char * proto, uint16_t port, const char * instance_name )
        {
            mdns_service_add( NULL, service, proto, port, nullptr, 0 );
            mdns_service_instance_name_set( service, proto, instance_name );
        }
    };
}    // namespace AsnPlus::Esp32

#endif

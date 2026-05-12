#pragma once

#include <sys/time.h>

#include "esp_sntp.h"

#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

namespace AsnPlus::Wifi
{
    /**
     * @brief Owns the full SNTP lifecycle: initialization, sync callback, and
     *        network-triggered restart.
     *
     *        Call initialize() once at startup (before any network is available).
     *        Call notifyNetworkAvailable() each time a network interface connects
     *        so that sync happens immediately instead of waiting for the next
     *        polling interval (ESP-IDF default: 1 h).
     *
     * @param server  NTP server hostname, e.g. "pool.ntp.org".
     * @param onSync  Optional delegate called with the synced epoch in
     *                milliseconds when SNTP successfully updates the clock.
     */
    class Sntp
    {
    public:
        Sntp( const char * server, Delegate< void( uint64_t ) > onSync = {} ) :
            _server( server ),
            _onSync( onSync )
        {
        }

        bool initialize()
        {
            Log::info( "Initializing (server=%s)", _server );

            _instance = this;

            esp_sntp_setoperatingmode( SNTP_OPMODE_POLL );
            esp_sntp_setservername( 0, _server );
            esp_sntp_set_sync_mode( SNTP_SYNC_MODE_IMMED );
            esp_sntp_set_time_sync_notification_cb( _sntpCallback );
            esp_sntp_init();

            Log::info( "Initialized — waiting for network to trigger first sync" );
            return true;
        }

        /**
         * @brief Call whenever a network interface becomes available.
         *        Restarts SNTP so the device syncs immediately.
         *        No-op after the first successful sync.
         */
        void notifyNetworkAvailable()
        {
            if ( _synced )
            {
                Log::debug( "Network available — already synced, skipping restart" );
                return;
            }

            Log::info( "Network available — restarting SNTP for immediate sync" );
            esp_sntp_restart();
        }

        bool isTimeSynchronized() const
        {
            return esp_sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED;
        }

    private:
        static constexpr const char TAG[] = "Sntp";
        using Log                         = Logger< 3, TAG >;    // info-level always visible

        const char *                 _server;
        Delegate< void( uint64_t ) > _onSync;
        bool                         _synced = false;

        static Sntp * _instance;

        static void _sntpCallback( struct timeval * tv )
        {
            if ( _instance == nullptr ) return;

            _instance->_synced      = true;

            const uint64_t epoch_ms = static_cast< uint64_t >( tv->tv_sec ) * 1000ULL +
                                      static_cast< uint64_t >( tv->tv_usec ) / 1000ULL;

            Log::info( "Synchronized — epoch=%llu ms", epoch_ms );

            if ( _instance->_onSync.is_valid() ) _instance->_onSync( epoch_ms );
        }
    };

    inline Sntp * Sntp::_instance = nullptr;

}    // namespace AsnPlus::Wifi

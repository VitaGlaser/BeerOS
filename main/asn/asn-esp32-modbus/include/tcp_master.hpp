#pragma once

#include "asn_module_config.hpp"

#include "esp_modbus_common.h"
#include "esp_modbus_master.h"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/timer.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-esp32-wifi/old/wifi_manager.hpp"

#include "components/external_data/source.hpp"

#include "master.hpp"
#include "structs.hpp"

namespace AsnPlus::Modbus
{
    class TcpMaster : public Master
    {
    public:
        static constexpr u32 MAX_SLAVES = 4;

        struct SlaveTable
        {
            struct Entry
            {
                static constexpr u32 IP_MAX_LEN = 64;

                uint8_t deviceId                = 0;
                char    ipAddress[ IP_MAX_LEN ] {};
            };

            struct Registration
            {
                static constexpr uint8_t MAX_ID_LENGTH = 64;

                uint64_t timestamp                     = 0;
                uint8_t  deviceId                      = 0;

                char id[ MAX_ID_LENGTH ] { '\0' };
            };

            static constexpr u32 MAX_REGISTRATIONS          = MAX_SLAVES;

            uint64_t     timestamp                          = 0;
            Entry        entries[ MAX_SLAVES ]              = {};
            Registration registrations[ MAX_REGISTRATIONS ] = {};
            u32          registrationCount                  = 0;

            void registerSource( uint8_t deviceId, char * id )
            {
                if ( registrationCount >= MAX_REGISTRATIONS ) return;
                for ( int i = 0; i < MAX_REGISTRATIONS; i++ )
                {
                    if ( registrations[ i ].id[ 0 ] == '\0' )
                    {
                        registrations[ i ] = { Esp32::Rtc::get_time_milis(), deviceId, {} };
                        strncpy( registrations[ i ].id, id, Registration::MAX_ID_LENGTH - 1 );
                        registrationCount++;
                        break;
                    }
                }
                timestamp = Esp32::Rtc::get_time_milis();
            }

            void unregisterSource( uint8_t deviceId )
            {
                for ( int i = 0; i < MAX_REGISTRATIONS; i++ )
                {
                    if ( registrations[ i ].deviceId == deviceId )
                    {
                        registrations[ i ] = {};
                        registrationCount--;
                    }
                }
                timestamp = Esp32::Rtc::get_time_milis();
            }

            void rebuild()
            {
                memset( entries, 0, sizeof( entries ) );
                u32 idx = 0;
                for ( u32 i = 0; i < registrationCount && idx < MAX_SLAVES; i++ )
                {
                    if ( registrations[ i ].id[ 0 ] == '\0' )
                    {
                        continue;
                    }
                    memset( entries[ idx ].ipAddress, 0, Entry::IP_MAX_LEN );
                    entries[ idx ].deviceId = registrations[ i ].deviceId;
                    snprintf(
                        entries[ idx ].ipAddress,
                        Entry::IP_MAX_LEN,
                        "%u;%s;502",
                        registrations[ i ].deviceId,
                        registrations[ i ].id
                    );
                    idx++;
                }
            }
        };

        struct Config
        {
            uint16_t port              = 502;
            uint32_t responseTimeoutMs = 5000;
        };

        enum class State
        {
            NOT_INITIALIZED,
            WAIT_FOR_CONNECTION,
            INITIALIZE,
            RETRY_INITIALIZATION,
            RUNNING
        };

        TcpMaster( Config & config, SlaveTable & slaveTable, Wifi::WifiManager & wifiManager ) :
            _config( config ),
            _slaveTable( slaveTable ),
            _wifiManager( wifiManager )
        {
        }

        bool initialize()
        {
            _state = State::WAIT_FOR_CONNECTION;
            return true;
        }

        void deinitialize()
        {
            if ( _masterHandler == nullptr ) return;

            mbc_master_stop( _masterHandler );
            mbc_master_delete( _masterHandler );
            _masterHandler = nullptr;
        }

        bool reinitialize()
        {
            deinitialize();
            return _initialize();
        }

        bool isInitialized() const { return _masterHandler != nullptr; }

        void addNetif( void * netif ) { _netif = netif; }

        bool request( mb_param_request_t * request, void * value )
        {
            if ( _masterHandler == nullptr )
            {
                Log::error( "Master handler is null, cannot send request" );
                return false;
            }
            esp_err_t ret = mbc_master_send_request( _masterHandler, request, value );
            if ( ret != ESP_OK )
            {
                Log::error( "mbc_master_send_request failed (%d)", static_cast< int >( ret ) );
                return false;
            }
            return true;
        }

        // deviceId mirrors the index in automaticControlSources
        // Use (slaveIndex + 1) as the Modbus UID in mb_param_request_t
        int findSlaveIndex( uint8_t deviceId ) const
        {
            for ( u32 i = 0; i < _slaveCount; i++ )
            {
                if ( _slaveTable.entries[ i ].deviceId == deviceId ) return static_cast< int >( i );
            }
            return -1;
        }

        void poll()
        {
            State newState = _state;

            switch ( _state )
            {
                case State::NOT_INITIALIZED:
                    break;
                case State::WAIT_FOR_CONNECTION:
                    if ( _wifiManager.is_connected() )
                    {
                        newState = State::INITIALIZE;
                    }
                    break;
                case State::INITIALIZE:
                    addNetif( _wifiManager.getNetif() );
                    if ( _initialize() )
                    {
                        newState = State::RUNNING;
                    }
                    else
                    {
                        newState = State::RETRY_INITIALIZATION;
                        _initializeRetryTimer.start( INITIALIZATION_RETRY_INTERVAL_MS );
                    }
                    break;
                case State::RETRY_INITIALIZATION:
                    if ( _initializeRetryTimer.isElapsed() )
                    {
                        deinitialize();
                        newState = State::WAIT_FOR_CONNECTION;
                    }
                    break;
                case State::RUNNING:
                    if ( ! _wifiManager.is_connected() )
                    {
                        deinitialize();
                        newState = State::WAIT_FOR_CONNECTION;
                    }
                    if ( _slaveTable.timestamp != _lastSeenTableTimestamp )
                    {
                        deinitialize();
                        newState                = State::WAIT_FOR_CONNECTION;
                        _lastSeenTableTimestamp = _slaveTable.timestamp;
                    }
                    break;
            }
            _state = newState;
        }

    private:
        static constexpr const char TAG[]                          = "Modbus::TcpMaster";
        using Log                                                  = Logger< ModuleConfig::Modbus::LOG_LEVEL, TAG >;

        static constexpr uint32_t INITIALIZATION_RETRY_INTERVAL_MS = 5000;

        Config &            _config;
        SlaveTable &        _slaveTable;
        Wifi::WifiManager & _wifiManager;

        void *   _netif                     = nullptr;
        u32      _slaveCount                = 0;
        char *   _ipTable[ MAX_SLAVES + 1 ] = {};
        uint64_t _lastSeenTableTimestamp    = 0;

        State _state                        = State::WAIT_FOR_CONNECTION;
        Timer _initializeRetryTimer {};

        // TODO: Fix device parameters properly, this is just a temporary fix
        const mb_parameter_descriptor_t device_parameters[ 1 ] = {
            { 0,
             "Data_channel_0", "Volts",
             1, MB_PARAM_INPUT,
             1020, 2,
             0, PARAM_TYPE_FLOAT,
             4, { 0, 0, 0 },
             PAR_PERMS_READ_WRITE_TRIGGER }
        };

        bool _initialize()
        {
            Log::info( "Initializing" );

            if ( _netif == nullptr )
            {
                Log::error( "Missing esp_netif pointer" );
                return false;
            }

            esp_log_level_set( "mbc_tcp.master", ESP_LOG_DEBUG );
            esp_log_level_set( "port_tcp_master", ESP_LOG_DEBUG );
            esp_log_level_set( "MB_TCP_MASTER", ESP_LOG_DEBUG );
            esp_log_level_set( "MB_CONTROLLER_MASTER", ESP_LOG_DEBUG );
            _slaveTable.rebuild();

            _slaveCount = 0;
            for ( u32 i = 0; i < MAX_SLAVES; i++ )
            {
                if ( _slaveTable.entries[ i ].ipAddress[ 0 ] == '\0' )
                {
                    break;
                }
                _ipTable[ i ] = _slaveTable.entries[ i ].ipAddress;
                _slaveCount++;
            }
            _ipTable[ _slaveCount ] = nullptr;

            if ( _slaveCount == 0 )
            {
                Log::error( "No slave entries configured" );
                return false;
            }

            mb_communication_info_t comm     = {};
            comm.tcp_opts.mode               = MB_TCP;
            comm.tcp_opts.port               = _config.port;
            comm.tcp_opts.uid                = 0;
            comm.tcp_opts.addr_type          = MB_IPV4;
            comm.tcp_opts.ip_addr_table      = reinterpret_cast< void * >( _ipTable );
            comm.tcp_opts.start_disconnected = true;
            comm.tcp_opts.response_tout_ms   = 2000;    //_config.responseTimeoutMs;
            comm.tcp_opts.ip_netif_ptr       = _netif;

            esp_err_t err                    = mbc_master_create_tcp( &comm, &_masterHandler );
            if ( err != ESP_OK || _masterHandler == nullptr )
            {
                Log::error( "mbc_master_create_tcp failed (%d)", static_cast< int >( err ) );
                return false;
            }

            err = mbc_master_start( _masterHandler );
            if ( err != ESP_OK )
            {
                Log::error( "mbc_master_start failed (%d)", static_cast< int >( err ) );
                return false;
            }

            err = mbc_master_set_descriptor( _masterHandler, &device_parameters[ 0 ], 1 );
            if ( err != ESP_OK )
            {
                Log::error( "mbc_master_set_descriptor failed (%d)", static_cast< int >( err ) );
                // return false;
            }

            _lastSeenTableTimestamp = _slaveTable.timestamp;

            Log::info( "Initialized with %d slave(s)", _slaveCount );
            return true;
        }
    };
}    // namespace AsnPlus::Modbus

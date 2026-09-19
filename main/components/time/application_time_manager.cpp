#include "application_time_manager.hpp"

#include <cstring>

namespace AsnPlus
{
    ApplicationTimeManager::ApplicationTimeManager(
        ISystemClock &       systemClock,
        IRtc *               rtc,
        IPersistentStorage & storage,
        TimeConfig &         timeConfig,
        TimeRuntime &        timeRuntime,
        TimeChangeRequest &  timeChangeRequest
    ) :
        _systemClock( systemClock ),
        _rtc( rtc ),
        _storage( storage ),
        _timeConfig( timeConfig ),
        _timeRuntime( timeRuntime ),
        _timeChangeRequest( timeChangeRequest )
    {
        _instance = this;
    }

    bool ApplicationTimeManager::initialize()
    {
        Log::info( "Initializing" );

        if ( ! _systemClock.initialize() )
        {
            Log::error( "Failed to initialize system clock" );
            return false;
        }

        _loadTimeConfig();

        if ( _timeConfig.timezone[ 0 ] != '\0' )
            _systemClock.setTimezone( _timeConfig.timezone );

        if ( _rtc != nullptr && ! _rtc->initialize() )
        {
            Log::warn( "RTC initialization failed; waiting for network time" );
        }
        else if ( _rtc != nullptr )
        {
            Log::warn( "RTC initialized successfully" );
        }
        else
        {
            Log::warn( "RTC is not configured" );
        }

        if ( _rtc != nullptr )
        {
            _syncTimeFromRtc();
        }

        _lastTimeConfigTimestamp = _timeConfig.timestamp;
        _initialized              = true;
        _updateRuntime();

        Log::info( "Initialized (utcEpochMs=%llu)", _timeRuntime.utcEpochMs );
        return true;
    }

    void ApplicationTimeManager::poll()
    {
        if ( ! _initialized ) return;

        _systemClock.poll();
        if ( _rtc != nullptr ) _rtc->poll();

        _applyTimeChangeRequest();

        if ( _timeConfig.timestamp != _lastTimeConfigTimestamp )
            _applyTimeConfig();

        _updateRuntime();
    }

    void ApplicationTimeManager::onNetworkTimeSync( uint64_t epochMs )
    {
        Time synchronizedTime {};
        synchronizedTime.fromEpochMillis( epochMs );
        if ( ! synchronizedTime.isValid() )
        {
            Log::warn( "Ignoring invalid network time: epoch=%llu", epochMs );
            return;
        }

        _setSystemAndRtc( synchronizedTime );
        _updateRuntime();
        Log::info( "Synchronized system clock and RTC: %u-%02u-%02u %02u:%02u:%02u",
                   synchronizedTime.year,
                   synchronizedTime.month,
                   synchronizedTime.day,
                   synchronizedTime.hour,
                   synchronizedTime.minute,
                   synchronizedTime.second );
    }

    void ApplicationTimeManager::_loadTimeConfig()
    {
        size_t length = sizeof( _timeConfig );
        if ( ! _storage.load( TIME_CONFIG_NVS_KEY, reinterpret_cast< uint8_t * >( &_timeConfig ), &length ) )
        {
            Log::info( "No saved time configuration" );
            return;
        }

        if ( length != sizeof( _timeConfig ) )
        {
            Log::warn( "Ignoring invalid time configuration size: %u", static_cast< unsigned >( length ) );
            memset( &_timeConfig, 0, sizeof( _timeConfig ) );
            return;
        }

        _timeConfig.timezone[ TimeConfig::TIMEZONE_LENGTH - 1 ]     = '\0';
        _timeConfig.timezoneName[ TimeConfig::TIMEZONE_NAME_LENGTH - 1 ] = '\0';
    }

    void ApplicationTimeManager::_saveTimeConfig()
    {
        if ( ! _storage.store(
                 TIME_CONFIG_NVS_KEY,
                 reinterpret_cast< const uint8_t * >( &_timeConfig ),
                 sizeof( _timeConfig )
             ) || ! _storage.commit() )
        {
            Log::error( "Failed to save time configuration" );
        }
    }

    void ApplicationTimeManager::_syncTimeFromRtc()
    {
        if ( _rtc == nullptr ) return;

        const Time rtcTime = _rtc->getUtc();
        if ( ! rtcTime.isValid() )
        {
            Log::warn( "RTC time is invalid" );
            return;
        }

        _systemClock.setUtc( rtcTime );
        Log::warn( "System time restored from RTC: %llu", rtcTime.toEpochMillis() );
    }

    void ApplicationTimeManager::_applyTimeChangeRequest()
    {
        if ( _timeChangeRequest.utcEpochMs == 0 ) return;

        Time requestedTime {};
        requestedTime.fromEpochMillis( _timeChangeRequest.utcEpochMs );
        if ( requestedTime.isValid() )
        {
            _setSystemAndRtc( requestedTime );
            Log::info( "Time changed manually: %llu", _timeChangeRequest.utcEpochMs );
        }
        else
        {
            Log::warn( "Ignoring invalid time change request: %llu", _timeChangeRequest.utcEpochMs );
        }

        _timeChangeRequest.utcEpochMs = 0;
    }

    void ApplicationTimeManager::_applyTimeConfig()
    {
        if ( _timeConfig.timezone[ 0 ] != '\0' )
            _systemClock.setTimezone( _timeConfig.timezone );

        _saveTimeConfig();
        _lastTimeConfigTimestamp = _timeConfig.timestamp;
    }

    void ApplicationTimeManager::_setSystemAndRtc( const Time & utc )
    {
        if ( ! utc.isValid() ) return;

        _systemClock.setUtc( utc );
        if ( _rtc != nullptr )
        {
            _rtc->setUtc( utc );
            Log::warn( "RTC updated from system time: %llu", utc.toEpochMillis() );
        }
    }

    void ApplicationTimeManager::_updateRuntime()
    {
        const Time utc = _systemClock.getUtc();
        _timeRuntime.utcEpochMs = utc.isValid() ? utc.toEpochMillis() : 0;
    }
}    // namespace AsnPlus
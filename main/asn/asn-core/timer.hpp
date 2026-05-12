#pragma once

#include "asn/asn-core/utils.hpp"

namespace AsnPlus
{
    struct MsClock
    {
        using Time = uint32_t;

        static Time now() { return Utils::getMs(); }
    };

    template< typename CLOCK = MsClock >
    class Timer
    {
    public:
        using Time  = typename CLOCK::Time;

        void start( Time duration )
        {
            _active      = true;
            _duration    = duration;
            _startTime   = CLOCK::now();
            _elapsedTime = 0;
        }

        void resume()
        {
            if ( _active ) return;
            Time remaining = _duration - _elapsedTime;
            if ( remaining > 0 )
            {
                start( remaining );
            }
        }

        void pause()
        {
            if ( ! _active ) return;
            _elapsedTime = elapsedTime();
            _active      = false;
        }

        bool isPaused() const { return ! _active && ( _elapsedTime < _duration ); }

        bool isElapsed() const
        {
            Time timePassed = _active ? elapsedTime() : _elapsedTime;
            return timePassed >= _duration;
        }

        bool isRunning() const { return _active && ( elapsedTime() < _duration ); }

        Time elapsedTime() const
        {
            if ( ! _active ) return _elapsedTime;
            return CLOCK::now() - _startTime;
        }

    private:
        static constexpr const char TAG[] = "Timer";

        bool _active                      = false;
        Time _startTime                   = 0;
        Time _duration                    = 0;
        Time _elapsedTime                 = 0;
    };

}    // namespace AsnPlus

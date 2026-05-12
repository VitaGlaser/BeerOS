#pragma once

namespace AsnPlus
{
    class Flag
    {
    public:
        bool value;

        void set() { value = true; }

        void set_if( bool condition )
        {
            if ( condition ) set();
        }

        void clear() { value = false; }

        bool operator()()
        {
            bool result = value;
            value       = false;
            return result;
        }
    };
}    // namespace AsnPlus

#pragma once

#include <cstdint>

namespace AsnPlus ::Expander
{
    class Transport
    {
    public:
        Transport()                                                       = default;
        ~Transport()                                                      = default;

        Transport( const Transport & )                                    = delete;
        Transport( Transport && )                                         = delete;
        Transport & operator=( const Transport & )                        = delete;
        Transport & operator=( Transport && )                             = delete;

        virtual uint16_t readRegister( uint16_t address )                 = 0;
        virtual void     writeRegister( uint16_t address, uint16_t data ) = 0;

    protected:
        static constexpr uint32_t DEFAULT_TIMEOUT_MS = 1000;

    private:
    };

}    // namespace AsnPlus::Expander

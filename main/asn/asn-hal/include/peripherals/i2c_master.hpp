#pragma once

#include "asn/asn-core/types.hpp"

namespace AsnPlus
{
    class II2cMaster
    {
    public:
        struct Config
        {
            uint32_t frequency;
        };

        static constexpr uint32_t DEFAULT_TIMEOUT_MS = 100;

        II2cMaster( Config & config ) : _config( config ) {}

        virtual bool initialize() = 0;

        virtual int32_t
            write( uint8_t deviceAddress, const uint8_t * data, size_t len, uint32_t timeout = DEFAULT_TIMEOUT_MS ) = 0;
        virtual int32_t
            read( uint8_t deviceAddress, uint8_t * data, size_t len, uint32_t timeout = DEFAULT_TIMEOUT_MS ) = 0;
        virtual int32_t transfer(
            uint8_t         deviceAddress,
            const uint8_t * writeData,
            size_t          writeLen,
            uint8_t *       readData,
            size_t          readLen,
            uint32_t        timeout = DEFAULT_TIMEOUT_MS
        ) = 0;

    protected:
        static constexpr const char TAG[] = "II2cMaster";
        Config &                    _config;

    private:
    };
}    // namespace AsnPlus
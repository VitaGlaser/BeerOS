#pragma once

#include "asn/asn-core/types.hpp"

#include "peripherals/gpio.hpp"

namespace AsnPlus
{
    class ISpiMaster
    {
    public:
        struct Config
        {
            uint32_t frequency;
        };

        ISpiMaster( Config & config ) : _config( config ) {}

        virtual bool initialize()                                                                     = 0;

        virtual int32_t write( uint8_t slaveId, const uint8_t * data, size_t len, uint32_t timeout ) = 0;
        virtual int32_t read( uint8_t slaveId, uint8_t * data, size_t len, uint32_t timeout )        = 0;
        virtual int32_t transfer(
            uint8_t         slaveId,
            const uint8_t * writeData,
            size_t          writeLen,
            uint8_t *       readData,
            size_t          readLen,
            uint32_t        timeout
        ) = 0;

    protected:
        static constexpr const char TAG[] = "ISpiMaster";
        Config &                    _config;

        virtual void _selectSlave( uint8_t slaveId )   = 0;
        virtual void _deselectSlave( uint8_t slaveId ) = 0;

    private:
    };
}    // namespace AsnPlus
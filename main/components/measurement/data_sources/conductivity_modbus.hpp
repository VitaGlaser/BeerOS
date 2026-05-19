#pragma once

#include "program/config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

#include "asn/asn-hal/include/time_manager/time_manager.hpp"

#include "asn/asn-esp32-modbus/include/master.hpp"

#include "base.hpp"

namespace AsnPlus::DataSource
{
    class ConductivityModbus : public Base
    {
    public:
        ConductivityModbus( Modbus::RtuMaster & modbusMaster ) : Base(), _modbusMaster( modbusMaster ) {}

        bool initialize() override
        {
            Log::debug( "Initializing" );

            Log::info( "Initialized" );
            return true;
        }

        void poll() override
        {
            uint16_t           value = 0;
            mb_param_request_t req   = {
                .slave_addr = static_cast< uint8_t >( _id ),
                .command    = static_cast< uint8_t >( Modbus::Commands::READ_INPUT_REGISTERS ),
                .reg_start  = 1,
                .reg_size   = 1
            };

            if ( ! _modbusMaster.request( &req, &value ) )
            {
                Log::error( "Failed to read input registers (ID: %llu)", _id );
                _clearSample();
                return;
            }

            uint64_t ts  = TimeManager::instance().getUtcTime().toEpochMillis();
            uint32_t val = static_cast< uint32_t >( value );
            _writeSample( ts, val );

            Log::debug( "Polled Modbus conductivity sensor (ID: %llu): %u (%llu ms)", _id, val, ts );
        }

    private:
        static constexpr const char TAG[] = "ConductivityModbusDataSource";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_DATA_SOURCES, TAG >;

        Modbus::RtuMaster & _modbusMaster;
    };
}    // namespace AsnPlus::DataSource

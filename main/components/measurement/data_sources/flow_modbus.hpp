#pragma once

#include "program/config.hpp"

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/utils.hpp"

#include "asn/asn-esp32-modbus/master.hpp"

#include "base.hpp"

namespace AsnPlus::DataSource
{
    class FlowModbus : public Base
    {
    public:
        FlowModbus( Modbus::RtuMaster & modbusMaster ) : Base(), _modbusMaster( modbusMaster ) {}

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
                .reg_start  = 0,
                .reg_size   = 1
            };

            if ( ! _modbusMaster.request( &req, &value ) )
            {
                Log::error( "Failed to read input registers" );
                _value = 0;
                return;
            }

            _value = static_cast< uint32_t >( value );
        }

    private:
        static constexpr const char TAG[] = "FlowModbusDataSource";
        using Log                         = Logger< ProjectConfig::LOG_LEVEL_DATA_SOURCES, TAG >;

        Modbus::RtuMaster & _modbusMaster;
    };
}    // namespace AsnPlus::DataSource

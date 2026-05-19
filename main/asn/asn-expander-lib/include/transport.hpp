#pragma once

#include <cstddef>
#include <cstdint>

namespace AsnPlus ::Expander
{
class Transport
    {
    public:
        Transport()                                                                               = default;
        ~Transport()                                                                              = default;

        Transport( const Transport & )                                                            = delete;
        Transport( Transport && )                                                                 = delete;
        Transport & operator=( const Transport & )                                                = delete;
        Transport & operator=( Transport && )                                                     = delete;

        virtual bool readRegister( uint16_t address, uint16_t & data )                            = 0;
        virtual bool readRegisters( uint16_t startAddress, uint16_t * data, size_t count )        = 0;
        virtual bool writeRegister( uint16_t address, uint16_t data )                             = 0;
        virtual bool writeRegisters( uint16_t startAddress, const uint16_t * data, size_t count ) = 0;

        virtual bool synchronizeWithBootlaoder() { return true; }

        virtual bool startCommand()                                  = 0;
        // virtual bool              readStatus( uint8_t & status )                  = 0;
        virtual bool sendData( const uint8_t * data, size_t length ) = 0;
        virtual bool receiveData( uint8_t * buffer, size_t length )  = 0;
        virtual bool waitAck()                                       = 0;

        virtual bool useNoStretchCommands() const { return false; }
    };

}    // namespace AsnPlus::Expander

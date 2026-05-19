#pragma once

#include "asn/asn-core/etl/delegate.h"
#include "asn/asn-core/etl/intrusive_forward_list.h"
#include "asn/asn-core/etl/intrusive_links.h"
#include "asn/asn-core/logger.hpp"
#include "transport.hpp"
#include <cstdint>

namespace AsnPlus::Expander
{
    using ILink = etl::forward_link< 0 >;

    class U16Register;

    class IRegisterContext
    {
    public:
        IRegisterContext( Transport & transport ) : _transport( transport ) {}

        ~IRegisterContext()                                      = default;

        IRegisterContext( const IRegisterContext & )             = delete;
        IRegisterContext( IRegisterContext && )                  = delete;
        IRegisterContext & operator=( const IRegisterContext & ) = delete;
        IRegisterContext & operator=( IRegisterContext && )      = delete;

        uint16_t readRegister( uint16_t address ) 
        {
            uint16_t value = 0;
            bool ok = _transport.readRegister( address, value );
            if (!ok) {
                //TODO: error handling
            }
            return value;  
        }

        void writeRegister( uint16_t address, uint16_t data ) 
        { 
            bool ok = _transport.writeRegister( address, data ); 
            if (!ok) {
                // TODO: error handling
            }
        }

        virtual void attach( U16Register & reg ) = 0;

    protected:
        etl::intrusive_forward_list< U16Register, ILink > _registers;

    private:
        Transport & _transport;
    };

    class RegisterContext;

    class U16Register : public ILink
    {
    public:
        U16Register( IRegisterContext & context, uint16_t address ) :
            value( 0 ),
            _address( address ),
            _context( context )
        {
            context.attach( *this );
        }

        U16Register( RegisterContext & context, uint16_t address );

        U16Register( IRegisterContext & context, uint16_t baseAddress, uint16_t address ) :
            U16Register( context, baseAddress + address )
        {
        }

        U16Register( RegisterContext & context, uint16_t baseAddress, uint16_t address );

        void load() { value = _context.readRegister( _address ); }

        void commit() { _context.writeRegister( _address, value ); }

        uint16_t fullAddress() { return _address; }

        uint16_t rawValue() { return value; }

        void printRaw() { Log::info( "[%04X]: %04X", _address, value ); }

    protected:
        uint16_t value;

    private:
        static constexpr const char TAG[] = "U16Register";
        using Log                         = Logger< 0, TAG >;

        uint16_t           _address;
        IRegisterContext & _context;
    };

    class RegisterContext : public IRegisterContext
    {
        using IRegisterContext::IRegisterContext;

    public:
        void attach( U16Register & reg ) override { _registers.push_front( reg ); }

        void iterateRegisters( etl::delegate< void( U16Register & ) > callback )
        {
            for ( auto & reg : _registers ) callback( reg );
        }
    };

    inline U16Register::U16Register( RegisterContext & context, uint16_t address ) :
        U16Register( static_cast< IRegisterContext & >( context ), address )
    {
    }

    inline U16Register::U16Register( RegisterContext & context, uint16_t baseAddress, uint16_t address ) :
        U16Register( static_cast< IRegisterContext & >( context ), baseAddress, address )
    {
    }
}    // namespace AsnPlus::Expander

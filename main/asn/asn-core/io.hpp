#ifndef _ASNPLUS_COMMON_IO_HPP
#define _ASNPLUS_COMMON_IO_HPP

#include "types.hpp"

namespace AsnPlus
{
    template< typename VALUE >
    class Input
    {
    public:
        VALUE read() { return __input_read(); }

        operator VALUE() { return read(); }

    protected:
        virtual VALUE __input_read() { return {}; }
    };

    template< typename VALUE >
    class Output
    {
    public:
        void write( VALUE value ) { __output_write( value ); }

        VALUE read() { return __output_read(); }

        void operator=( VALUE value ) { return write( value ); }

    protected:
        virtual void __output_write( VALUE value ) {}

        virtual VALUE __output_read() { return {}; }
    };
};    // namespace AsnPlus

#endif

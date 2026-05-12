#pragma once

#include "asn/asn-core/array.hpp"
#include "asn/asn-core/string.hpp"
#include "asn/asn-core/to_string.hpp"

#include "asn/asn-core/vector.hpp"

namespace AsnPlus
{
    class CommandBuilder
    {
    public:
        explicit CommandBuilder( IVector< uint8_t > & targetBuffer, bool & requiresClear, StringView commandBase ) :
            _hasArguments( false ),
            _requiresClear( requiresClear ),
            _commandBase( commandBase ),
            _buffer( targetBuffer )
        {
        }

        CommandBuilder & read();
        CommandBuilder & test();
        CommandBuilder & set();
        CommandBuilder & arg( ArrayView< uint8_t > argument );
        CommandBuilder & arg( int value );
        CommandBuilder & argStr( StringView argument );
        CommandBuilder & argStr( int value );
        void             checkBaseInsertion();

    private:
        bool                 _hasArguments;
        bool &               _requiresClear;
        StringView           _commandBase;
        IVector< uint8_t > & _buffer;
    };
}    // namespace AsnPlus

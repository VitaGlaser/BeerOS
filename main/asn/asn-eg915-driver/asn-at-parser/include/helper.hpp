#pragma once

#include "asn/asn-core/string.hpp"
#include "asn/asn-core/etl/to_arithmetic.h"

namespace AsnPlus::At
{
    inline StringView stripUrcPrefix( StringView urc )
    {
        return trim_view_whitespace( urc.substr( urc.find( ":" ) + 1 ) );
    }

    template< typename T >
    bool parseNextParameter( StringView & data, T & out )
    {
        if ( data.empty() ) return false;
        StringView toParse = data;
        auto       pos     = toParse.find( "," );
        if ( pos != StringView::npos )
        {
            toParse = toParse.substr( 0, pos );
        }
        auto res = etl::to_arithmetic< T >( toParse );
        if ( ! res.has_value() )
        {
            return false;
        }
        out  = res.value();
        data = ( pos == StringView::npos ) ? StringView() : data.substr( pos + 1 );
        return true;
    }
}    // namespace AsnPlus::At

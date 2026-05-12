#pragma once

#include "asn/asn-core/array.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn_module_config.hpp"

#include <cstddef>
#include <cstdio>
#include <iomanip>

namespace AsnPlus::At
{
    constexpr auto NumError = etl::numeric_limits< size_t >::max();

    template< typename Iterator >
    size_t findFirst( Iterator begin, Iterator end, const char * search )
    {
        size_t searchLen = strlen( search );
        size_t dataLen   = static_cast< size_t >( end - begin );

        if ( searchLen == 0 || dataLen < searchLen ) return NumError;

        for ( size_t i = 0; i <= dataLen - searchLen; ++i )
        {
            bool match = true;
            for ( size_t j = 0; j < searchLen; ++j )
            {
                if ( static_cast< uint8_t >( search[ j ] ) != static_cast< uint8_t >( *( begin + i + j ) ) )
                {
                    match = false;
                    break;
                }
            }
            if ( match ) return i;
        }
        return NumError;
    }

    template< typename Iterator >
    Iterator findFirstIterator( Iterator begin, Iterator end, const char * search )
    {
        size_t pos = findFirst( begin, end, search );
        if ( pos == NumError ) return end;
        return begin + pos;
    }

}    // namespace AsnPlus::At

template< typename Iterator >
void printVector( Iterator begin, Iterator end )
{
    for ( auto it = begin; it != end; ++it )
    {
        if ( *it != ' ' && *it != '\t' && *it != '\n' && *it != '\r' )
        {
            printf( "%c", static_cast< char >( *it ) );
        }
        else
        {
            char tmp[ 5 ];
            std::snprintf( tmp, sizeof( tmp ), "\\x%02X", static_cast< unsigned int >( *it ) & 0xFF );
            printf( "%s", tmp );
        }
    }
}

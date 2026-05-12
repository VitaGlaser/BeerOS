#pragma once

#include "etl/to_arithmetic.h"
#include "string.hpp"

namespace AsnPlus
{
    class Parser
    {
    public:
        static bool parse( uint32_t & output, StringView & input ) { return parse_number( output, input ); }

        static bool parse( uint8_t & output, StringView & input ) { return parse_number( output, input ); }

        static bool parse( float & output, StringView & input ) { return parse_number( output, input ); }

        static bool parse( bool & output, StringView & input )
        {
            output = input == "true";
            return true;
        }

        template< typename OUTPUT >
        static OUTPUT parse( StringView & input )
        {
            OUTPUT result {};
            parse( result, input );
            return result;
        }

        template< typename OUTPUT >
        static bool parse_number( OUTPUT & output, StringView & input )
        {
            auto result = etl::to_arithmetic< uint32_t >( input );
            output      = result.value();

            return result.has_value();
        }
    };

    template<>
    StringView Parser::parse< StringView >( StringView & input )
    {
        return input;
    }
}    // namespace AsnPlus

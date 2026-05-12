#pragma once

#include "generic.hpp"
#include "string.hpp"

namespace AsnPlus
{
    class EnumStrings
    {
    public:
        template< typename ENUM >
        using Mapping = std::initializer_list< Pair< ENUM, const char * > >;

        template< typename VALUE >
        using Result = TResult< VALUE, bool, true >;

        template< typename ENUM >
        static constexpr Mapping< ENUM > mapping();

        template< typename ENUM >
        static constexpr Result< StringView > to_string( ENUM value )
        {
            for ( auto & pair : EnumStrings::mapping< ENUM >() )
            {
                if ( pair.first == value ) return StringView { pair.second };
            }
            return false;
        }

        template< typename ENUM >
        static constexpr Result< ENUM > from_string( StringView string )
        {
            for ( auto & pair : EnumStrings::mapping< ENUM >() )
            {
                if ( pair.second == string ) return pair.first;
            }
            return false;
        }
    };

    template< typename T, typename = void >
    struct has_to_string : std::false_type
    {
    };

    template< typename T >
    struct has_to_string< T, std::void_t< decltype( std::declval< T >().to_string( std::declval< IString & >() ) ) > > :
        std::true_type
    {
    };

    template< typename VALUE >
    void _serialize( VALUE const & to_serialize, IString & result, std::false_type, std::false_type )
    {
        etl::to_string( to_serialize, result, true );
    }

    inline void _serialize( const char * to_serialize, IString & result, std::false_type, std::false_type )
    {
        etl::to_string( StringView { to_serialize }, result, true );
    }

    template< size_t N >
    void _serialize( const String< N > & to_append, IString & result, std::false_type, std::false_type )
    {
        etl::to_string( (IString &) to_append, result, true );
    }

    inline void _serialize( bool to_serialize, IString & result, std::false_type, std::false_type )
    {
        Format format { 10, 0, 0, false, false, true, false, ' ' };
        etl::to_string( to_serialize, result, format, true );
    }

    inline void _serialize( float to_serialize, IString & result, std::false_type, std::false_type )
    {
        Format format { 10, 0, 2, false, false, false, false, ' ' };
        etl::to_string( to_serialize, result, format, true );
    }

    inline void _serialize( double to_serialize, IString & result, std::false_type, std::false_type )
    {
        Format format { 10, 0, 2, false, false, false, false, ' ' };
        etl::to_string( to_serialize, result, format, true );
    }

    template< typename VALUE >
    void _serialize( VALUE const & to_serialize, IString & result, std::true_type, std::false_type )
    {
        auto serialized = EnumStrings::to_string< VALUE >( to_serialize ).value_or( "[invalid]" );
        etl::to_string( serialized, result, true );
    }

    template< typename VALUE >
    void _serialize( VALUE const & to_serialize, IString & result, std::false_type, std::true_type )
    {
        to_serialize.to_string( result );
    }

    template< typename VALUE >
    void serialize( VALUE const & to_serialize, IString & result )
    {
        _serialize( to_serialize, result, std::is_enum< VALUE >(), has_to_string< VALUE >() );
    }

    template< typename VALUE >
    struct Formatted
    {
        VALUE  value;
        Format format;

        void to_string( IString & result ) const { etl::to_string( value, result, format, true ); }
    };

    inline Formatted< void * > Ptr( void * value )
    {
        return {
            value, { 16, 8, 0, false, false, false, true, '0' }
        };
    }

    template< typename VALUE >
    Formatted< VALUE > Hex( VALUE value )
    {
        return Formatted< VALUE > {
            value, Format { 16, 0, 0, false, false, false, true, ' ' }
        };
    }

    inline Formatted< StringView > Str( StringView value )
    {
        return { value, {} };
    }

    template< size_t SIZE, typename T >
    String< SIZE > ToString( const T & value )
    {
        String< SIZE > result;
        serialize( value, result );

        return result;
    }
}    // namespace AsnPlus

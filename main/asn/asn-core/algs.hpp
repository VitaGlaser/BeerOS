#pragma once

#include "etl/absolute.h"
#include "etl/algorithm.h"
#include "etl/correlation.h"
#include "etl/gcd.h"
#include "etl/lcm.h"
#include "etl/math.h"
#include "etl/negative.h"
#include "types.hpp"

namespace AsnPlus
{
    struct Alg
    {
        static u32 reverse32( u32 input )
        {
            return ( input & 0x00'00'00'FF ) << 24 | ( input & 0x00'00'FF'00 ) << 8 | ( input & 0x00'FF'00'00 ) >> 8 |
                ( input & 0xFF'00'00'00 ) >> 24;
        }

        template< typename T >
        static constexpr T bcd_decode( T bcd )
        {
            T decimal = 0;
            for ( int i = static_cast< int >( sizeof( T ) ) * 2 - 1; i >= 0; --i )
                decimal = static_cast< T >( decimal * 10 + ( ( bcd >> ( i * 4 ) ) & 0xF ) );
            return decimal;
        }

        template< typename T >
        static constexpr T bcd_encode( T value )
        {
            T bcd = 0;
            for ( u32 shift = 0; shift < sizeof( T ) * 8; shift += 4 )
            {
                bcd   = static_cast< T >( bcd + ( ( value % 10 ) << shift ) );
                value = static_cast< T >( value / 10 );
            }
            return bcd;
        }

        static u8 hex_decode( u8 hex_digit )
        {
            if ( (u8) ( hex_digit - '0' ) < 10 ) return hex_digit - '0';
            else if ( (u8) ( hex_digit - 'a' ) < 6 ) return hex_digit - 'a' + 10;
            else if ( (u8) ( hex_digit - 'A' ) < 6 ) return hex_digit - 'A' + 10;

            else return 0;
        }

        static u8 hex_encode( u8 value ) { return value < 10 ? value + '0' : value - 10 + 'a'; }

        template< typename T >
        static constexpr auto abs( T value )
        {
            return etl::absolute( value );
        }

        template< typename T >
        static constexpr auto abs_unsigned( T value )
        {
            return etl::absolute_unsigned( value );
        }

        template< typename T >
        static constexpr bool is_negative( T value )
        {
            return etl::is_negative( value );
        }

        // for floating-point numbers
        template< typename T >
        static constexpr bool is_nan( T value )
        {
            return etl::is_nan( value );
        }

        // for floating-point numbers
        template< typename T >
        static constexpr bool is_infinity( T value )
        {
            return etl::is_infinity( value );
        }

        // for floating-point numbers
        template< typename T >
        static constexpr bool is_zero( T value )
        {
            return etl::is_zero( value );
        }

        // for floating-point numbers
        template< typename T >
        static constexpr bool is_exactly_equal( T a, T b )
        {
            return etl::is_exactly_equal( a, b );
        }

        // The greatest common divisor
        template< typename T, typename... TRest >
        static constexpr auto gcd( T first, TRest... rest )
        {
            return etl::gcd( first, rest... );
        }

        // The least common multiple
        template< typename T, typename... TRest >
        static constexpr auto lcm( T first, TRest... rest )
        {
            return etl::lcm( first, rest... );
        }

        template< typename VALUE >
        static constexpr VALUE const & min( VALUE const & a, VALUE const & b )
        {
            return etl::min( a, b );
        }

        template< typename VALUE, typename COMPARE >
        static constexpr VALUE const & min( VALUE const & a, VALUE const & b, COMPARE compare )
        {
            return etl::min( a, b, compare );
        }

        template< typename VALUE >
        static constexpr VALUE const & max( VALUE const & a, VALUE const & b )
        {
            return etl::max( a, b );
        }

        template< typename VALUE, typename COMPARE >
        static constexpr VALUE const & max( VALUE const & a, VALUE const & b, COMPARE compare )
        {
            return etl::max( a, b, compare );
        }

        template< typename VALUE >
        static constexpr auto minmax( VALUE const & a, VALUE const & b )
        {
            return etl::minmax( a, b );
        }

        template< typename VALUE, typename PREDICATE >
        static constexpr auto minmax( VALUE const & a, VALUE const & b, PREDICATE predicate )
        {
            return etl::minmax( a, b, predicate );
        }

        template< typename VALUE, VALUE LOW, VALUE HIGH >
        static constexpr auto clamp( VALUE const & value )
        {
            return etl::clamp( value );
        }

        template< typename VALUE, VALUE LOW, VALUE HIGH, typename PREDICATE >
        static constexpr auto clamp( VALUE const & value, PREDICATE predicate )
        {
            return etl::clamp( value, predicate );
        }

        template< typename VALUE >
        static constexpr auto clamp( VALUE const & value, VALUE const & low, VALUE const & high )
        {
            return etl::clamp( value, low, high );
        }

        template< typename VALUE, typename COMPARE >
        static constexpr auto clamp( VALUE const & value, VALUE const & low, VALUE const & high, COMPARE compare )
        {
            return etl::clamp( value, low, high, compare );
        }

        template< typename VALUE, typename... VALUES >
        static constexpr auto multimin( VALUE const & value, VALUES const &... values )
        {
            return etl::multimin( value, values... );
        }

        template< typename COMPARE, typename VALUE, typename... VALUES >
        static constexpr auto multimin_compare( COMPARE compare, VALUE const & value, VALUES const &... values )
        {
            return etl::multimin_compare( compare, value, values... );
        }

        template< typename ITERATOR, typename... ITERATORS >
        static constexpr auto multimin_iter( ITERATOR const & iterator, ITERATORS const &... iterators )
        {
            return etl::multimin_iter( iterator, iterators... );
        }

        template< typename COMPARE, typename ITERATOR, typename... ITERATORS >
        static constexpr auto
            multimin_iter_compare( COMPARE compare, ITERATOR const & iterator, ITERATORS const &... iterators )
        {
            return etl::multimin_iter_compare( compare, iterator, iterators... );
        }

        template< typename VALUE, typename... VALUES >
        static constexpr auto multimax( VALUE const & value, VALUES const &... values )
        {
            return etl::multimax( value, values... );
        }

        template< typename COMPARE, typename VALUE, typename... VALUES >
        static constexpr auto multimax_compare( COMPARE compare, VALUE const & value, VALUES const &... values )
        {
            return etl::multimax_compare( compare, value, values... );
        }

        template< typename ITERATOR, typename... ITERATORS >
        static constexpr auto multimax_iter( ITERATOR const & iterator, ITERATORS const &... iterators )
        {
            return etl::multimax_iter( iterator, iterators... );
        }

        template< typename COMPARE, typename ITERATOR, typename... ITERATORS >
        static constexpr auto
            multimax_iter_compare( COMPARE compare, ITERATOR const & iterator, ITERATORS const &... iterators )
        {
            return etl::multimax_iter_compare( compare, iterator, iterators... );
        }

        template< typename ITERATOR_1, typename ITERATOR_2 >
        static constexpr auto iter_swap( ITERATOR_1 a, ITERATOR_2 b )
        {
            return etl::iter_swap( a, b );
        }
    };
}    // namespace AsnPlus

#pragma once

#include "etl/type_traits.h"
#include "string.hpp"
#include "to_string.hpp"

namespace AsnPlus
{
    template< typename T >
    struct AxisValues
    {
        static_assert( etl::is_arithmetic< T >::value, "AxisValues requires a numeric type" );

        T x {};
        T y {};
        T z {};

        AxisValues() = default;

        AxisValues( T x_, T y_, T z_ ) : x( x_ ), y( y_ ), z( z_ ) {}

        // Euclidean magnitude
        T magnitude() const { return sqrt( x * x + y * y + z * z ); }

        // Subtraction
        AxisValues< T > operator-( const AxisValues< T > & other ) const
        {
            return AxisValues< T >( x - other.x, y - other.y, z - other.z );
        }

        // Addition
        AxisValues< T > operator+( const AxisValues< T > & other ) const
        {
            return AxisValues< T >( x + other.x, y + other.y, z + other.z );
        }

        // Scalar division
        AxisValues< T > operator/( T scalar ) const { return AxisValues< T >( x / scalar, y / scalar, z / scalar ); }

        // String conversion
        String< 64 > to_string( int precision = 2 ) const
        {
            String< 64 > result;
            result += "X: ";
            result += to_string( x, precision );
            result += ", Y: ";
            result += to_string( y, precision );
            result += ", Z: ";
            result += to_string( z, precision );
            return result;
        }
    };
}    // namespace AsnPlus

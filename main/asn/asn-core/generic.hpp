#pragma once

#include "etl/type_traits.h"

namespace AsnPlus
{
    template< typename VALUE >
    static VALUE & default_value()
    {
        static VALUE value {};
        return value;
    }

    template< typename T >
    using enable_if_ref = typename etl::enable_if< etl::is_lvalue_reference< T >::value >;

    template< typename VALUE, typename = void >
    struct ValueOrRef
    {
        using ValueType = VALUE;
        ValueType value {};
    };

    template< typename VALUE >
    struct ValueOrRef< VALUE, typename enable_if_ref< VALUE >::type >
    {
        using ValueType   = etl::decay_t< VALUE >;
        ValueType & value = default_value< VALUE >();
    };

    template< typename T, typename = void >
    struct is_container : std::false_type
    {
    };

    template< typename T >
    struct is_container<
        T,
        std::void_t<
            decltype( std::declval< T >().begin() ),
            decltype( std::declval< T >().end() ),
            decltype( std::declval< T >().size() ) > > : std::true_type
    {
    };
}    // namespace AsnPlus

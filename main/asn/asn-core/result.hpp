#pragma once

#include "generic.hpp"

namespace AsnPlus
{
    template< typename VALUE, typename ERROR, ERROR _OK_VALUE = ERROR() >
    class TResult : public ValueOrRef< VALUE >
    {
    public:
        using Base                          = ValueOrRef< VALUE >;
        using ErrorType                     = ERROR;
        using ValueType                     = typename Base::ValueType;

        static constexpr ErrorType OK_VALUE = _OK_VALUE;
        ErrorType                  error;

        constexpr TResult( ErrorType error ) : Base {}, error { error } {}

        constexpr TResult( ValueType value ) : Base { value }, error { OK_VALUE } {}

        constexpr operator bool() const { return error == OK_VALUE; }

        constexpr operator ValueType() const { return Base::value; }

        // constexpr
        // operator ErrorType() const
        // {
        //     return error;
        // }

        constexpr ValueType value_or( ValueType default_value ) const
        {
            return error == OK_VALUE ? Base::value : default_value;
        }
    };
}    // namespace AsnPlus

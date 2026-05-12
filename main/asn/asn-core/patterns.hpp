#pragma once

#include "etl/observer.h"
#include "etl/overload.h"
#include "etl/singleton.h"
#include "etl/successor.h"
#include "etl/visitor.h"

namespace AsnPlus
{
    template< typename OBSERSVER, const size_t MAX_OBSERVERS >
    using Observable = etl::observable< OBSERVER, MAX_OBSERVERS >;

    template< typename T, typename... T_REST >
    using Observer = etl::observer< T, T_REST... >;

    template< typename... LAMBDA_TYPES >
    using LambdaOverload = etl::overload< LAMBDA_TYPES... >;

    template< typename T >
    using Singleton = etl::singleton< T >;

    template< typename T >
    using Successor = etl::successor< T >;

    template< typename T, typename... T_REST >
    using Visitable = etl::visitable< T, T_REST... >;

    template< typename T, typename... T_REST >
    using Visitor = etl::visitor< T, T_REST... >;
}    // namespace AsnPlus

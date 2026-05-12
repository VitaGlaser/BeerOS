#pragma once

#include "etl/combinations.h"
#include "etl/factorial.h"
#include "etl/fibonacci.h"
#include "etl/log.h"
#include "etl/math_constants.h"
#include "etl/permutations.h"
#include "etl/power.h"
#include "etl/sqrt.h"

namespace AsnPlus
{
    struct Constant : etl::math
    {
        template< size_t NUMBER, size_t BASE >
        static constexpr auto log()
        {
            return etl::log< NUMBER, BASE >::value;
        }

        template< size_t NUMBER >
        static constexpr auto log2()
        {
            return etl::log2< NUMBER >::value;
        }

        template< size_t NUMBER >
        static constexpr auto log10()
        {
            return etl::log10< NUMBER >::value;
        }

        template< size_t NUMBER, size_t POWER >
        static constexpr auto power()
        {
            return etl::power< NUMBER, POWER >::value;
        }

        template< size_t NUMBER >
        static constexpr auto power_of_2_above()
        {
            return etl::power_of_2_round_up< NUMBER >::value;
        }

        template< size_t NUMBER >
        static constexpr auto power_of_2_below()
        {
            return etl::power_of_2_round_down< NUMBER >::value;
        }

        template< size_t NUMBER >
        static constexpr auto is_power_of_2()
        {
            return etl::is_power_of_2< NUMBER >::value;
        }

        template< size_t NUMBER >
        static constexpr auto square_root()
        {
            return etl::sqrt< NUMBER >::value;
        }

        template< size_t NUMBER >
        static constexpr auto factorial()
        {
            return etl::factorial< NUMBER >::value;
        }

        template< size_t NUMBER >
        static constexpr auto fibonacci()
        {
            return etl::fibonacci< NUMBER >::value;
        }

        template< size_t SET_SIZE, size_t SUBSET_SIZE >
        static constexpr auto permutations()
        {
            return etl::permutations< SET_SIZE, SUBSET_SIZE >::value;
        }

        template< size_t SET_SIZE, size_t SUBSET_SIZE >
        static constexpr auto combinations()
        {
            return etl::combinations< SET_SIZE, SUBSET_SIZE >::value;
        }
    };
}    // namespace AsnPlus

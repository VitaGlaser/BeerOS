#pragma once

#include "etl/ratio.h"

namespace AsnPlus
{
    template< size_t NUMERATOR, size_t DENOMINATOR = 1UL >
    using Ratio = etl::ratio< NUMERATOR, DENOMINATOR >;

#if INT_MAX >= INT16_MAX
    using etl::centi;
    using etl::deca;
    using etl::deci;
    using etl::hecto;
    using etl::kilo;
    using etl::milli;
#endif

#if INT_MAX >= INT32_MAX
    using etl::giga;
    using etl::mega;
    using etl::micro;
    using etl::nano;
#endif

#if INT_MAX > INT32_MAX
    using etl::atto;
    using etl::exa;
    using etl::femto;
    using etl::peta;
    using etl::pico;
    using etl::tera;
    using etl::yocto;
    using etl::yotta;
    using etl::zepto;
    using etl::zetta;
#endif

    using etl::ratio_e;        // An approximation of e
    using etl::ratio_pi;       // An approximation of PI to 6 digits
    using etl::ratio_root2;    // An approximation of root 2
}    // namespace AsnPlus

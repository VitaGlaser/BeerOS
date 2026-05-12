#pragma once

#include "etl/pseudo_moving_average.h"

namespace AsnPlus
{
    /*
        https://www.etlcpp.com/pseudo_moving_average.html

        A moving average algorithm that continuously calculates an average value from a stream of samples.The sample
       size does not affect the size of the instantiated object. There is no overhead based on the number ofsamples as
       it simulates a window of N values from the current average.

        There are four variants of the algorithm; two for integral values and two for floating point. Each sub  variant
       allows theselection of compile time or run time sample size. The integral variant allows a compile time scaling
       factor to emulate fixed point arithmetic.

        If the current moving average is 5, then an equivalent sequence of samples (for a sample size of 9), that gives
       the sameaverage, would be 5, 5, 5, 5, 5, 5, 5, 5, 5 This means, to find the average when adding a new sample to a
       moving average that has a current value of 5, all weneed to do is multiply the current average by the sample size
       (9), add the new sample, and divide by the sample size +1 (10).

        EXAMPLE:

        Array< u32, 10 >            data    { 9, 1, 8, 2, 7, 3, 6, 4, 5 };
        PseudoAverage< u32, 10 >    average { 0 };      // 0 is the initial value

        for (u32 sample : data) average.add( sample );

        u32 result = average.value();
    */
    template< typename SAMPLE_TYPE, const size_t SAMPLE_COUNT, const size_t SCALING = 1 >
    using PseudoAverage = etl::pseudo_moving_average< SAMPLE_TYPE, SAMPLE_COUNT, SCALING >;
}    // namespace AsnPlus

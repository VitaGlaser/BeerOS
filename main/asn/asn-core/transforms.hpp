#pragma once

#include "etl/histogram.h"
#include "etl/invert.h"
#include "etl/limiter.h"
#include "etl/mean.h"
#include "etl/quantize.h"
#include "etl/rescale.h"
#include "etl/rms.h"
#include "etl/standard_deviation.h"
#include "etl/threshold.h"

namespace AsnPlus
{
    template<
        typename KEY,
        typename COUNT,
        size_t  MAX_SIZE,
        int32_t START_INDEX = etl::integral_limits< int32_t >::max >
    using Histogram = etl::histogram< KEY, COUNT, MAX_SIZE, START_INDEX >;

    template< typename KEY, typename COUNT, size_t MAX_SIZE >
    using SparseHistogram = etl::sparse_histogram< KEY, COUNT, MAX_SIZE >;

    template< typename INPUT >
    using Invert = etl::invert< INPUT >;

    template< typename INPUT, typename LIMITER = etl::private_limiter::limit< INPUT > >
    using Limiter = etl::limiter< INPUT, LIMITER >;

    template< typename INPUT, typename OUTPUT = INPUT >
    using Mean = etl::mean< INPUT, OUTPUT >;

    template< typename INPUT, typename COMPARE = etl::less< INPUT > >
    using Quantize = etl::quantize< INPUT, COMPARE >;

    template< typename INPUT, typename OUTPUT >
    using Rescale = etl::rescale< INPUT, OUTPUT >;

    template< typename INPUT, typename OUTPUT >
    using RootMeanSquare = etl::rms< INPUT, OUTPUT >;

    template< typename INPUT, typename COMPARE = etl::less< INPUT > >
    using Threshold = etl::threshold< INPUT, COMPARE >;

    template< bool USE_SAMPLE_VARIANCE, typename INPUT, typename OUTPUT = INPUT >
    using StandardDeviation = etl::standard_deviation< USE_SAMPLE_VARIANCE, INPUT, OUTPUT >;
}    // namespace AsnPlus

#pragma once

#include "etl/random.h"

namespace AsnPlus
{
    /*
        https://www.etlcpp.com/random.html

        Uses a 128bit XOR shift algorithm for producing a pseudo-random sequence of integers.
        The result is a 32 bit integer between 0 and 4,294,967,295 (2^32 - 1).

        EXAMPLE:

        RandomXorShift random {};
        u32 result1 = random();
        u32 result2 = random.range( 0, 10 );
    */
    using RandomXorShift = etl::random_xorshift;

    /*
        https://www.etlcpp.com/random.html

        Generates a 32 bit pseudo-random number using a linear congruent generator.
        The result is a 32 bit integer between 0 and 2,147,483,647 (2^31 - 1).

        EXAMPLE:

        RandomLCG random {};
        u32 result1 = random();
        u32 result2 = random.range( 0, 10 );
    */
    using RandomLCG      = etl::random_lcg;

    /*
        https://www.etlcpp.com/random.html

        Generates a 32 bit pseudo-random number using a combined linear congruent generator.
        The result is a 32 bit integer between 0 and 2,147,483,647 (2^31 - 1).

        EXAMPLE:

        RandomCLCG random {};
        u32 result1 = random();
        u32 result2 = random.range( 0, 10 );
    */
    using RandomCLCG     = etl::random_clcg;

    /*
        https://www.etlcpp.com/random.html

        Generates a 32 bit pseudo-random number using a permuted congruential generator algorithm.
        The result is a 32 bit integer between 1 and 4,294,967,295 (2^32 - 1)

        EXAMPLE:

        RandomPCG random {};
        u32 result1 = random();
        u32 result2 = random.range( 0, 10 );
    */
    using RandomPCG      = etl::random_pcg;

    /*
        https://www.etlcpp.com/random.html

        Generates a 32 bit pseudo-random number using a linear shift feedback register.
        The result is a 32 bit integer between 1 and 4,294,967,295 (2^32 - 1).
        The seed must not be zero. The output does not include zero.

        EXAMPLE:

        RandomLSFR random {};
        u32 result1 = random();
        u32 result2 = random.range( 0, 10 );
    */
    using RandomLSFR     = etl::random_lsfr;

    /*
        https://www.etlcpp.com/random.html

        Generates a 32 bit pseudo-random number using a multiply-with-carry algorithm.
        The result is a 32 bit integer between 1 and 4,294,967,295 (2^32 - 1).

        EXAMPLE:

        RandomMWC random {};
        u32 result1 = random();
        u32 result2 = random.range( 0, 10 );
    */
    using RandomMWC      = etl::random_mwc;

    /*
        https://www.etlcpp.com/random.html

        Generates a 32 bit pseudo-random number by applying a user supplied 32bit hash to a counter.
        The hash must implement void add(uint8_t) and uint8_t value() member functions.

        EXAMPLE:

        struct CustomHash
        {
            void add( u8 next_value ) { ... }
            u32 value() { ... }
        };

        RandomHash< CustomHash > random {};
        u32 result1 = random();
        u32 result2 = random.range( 0, 10 );
    */
    template< typename HASH_ALG >
    using RandomHash = etl::random_hash< HASH_ALG >;
}    // namespace AsnPlus

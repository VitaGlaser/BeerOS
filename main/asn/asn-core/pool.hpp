#pragma once

#include "etl/pool.h"

namespace AsnPlus
{
    // https://www.etlcpp.com/pool.html

    template< typename ITEM_TYPE, const size_t MAX_SIZE >
    using Pool = etl::pool< ITEM_TYPE, MAX_SIZE >;

    template< size_t TYPE_SIZE, size_t ALIGNMENT, size_t SIZE >
    using GenericPool = etl::generic_pool< TYPE_SIZE, ALIGNMENT, SIZE >;

    using IPool       = etl::ipool;

    /*
        EXAMPLE

        class Data
        {
            ...
        };

        Pool< Data, 10 > data_pool;

        // Create
        Data * pdata = new (data_pool.allocate<Data>()) Data();

        // Destroy
        pdata->~Data();
        data_pool.release(pdata);
    */
}    // namespace AsnPlus

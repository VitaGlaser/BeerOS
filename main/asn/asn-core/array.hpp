#pragma once

#include "etl/multi_array.h"
#include "etl/array_view.h"

namespace AsnPlus
{
    template< typename ITEM_TYPE>
    using ArrayView = etl::array_view< ITEM_TYPE >;

    template< typename ITEM_TYPE, size_t... SIZES >
    using Array = etl::multi_array< ITEM_TYPE, SIZES... >;

    template< typename ITEM_TYPE, size_t SIZE >
    class ArrayUtil
    {
    public:
        Array< ITEM_TYPE, SIZE > & items;

        size_t find_index( ITEM_TYPE & to_find )
        {
            size_t i = 0;

            for ( auto item : items )
            {
                if ( item == to_find ) break;
                i++;
            }
            return i;
        }
    };

    template< typename ITEM_TYPE, size_t SIZE >
    ArrayUtil< ITEM_TYPE, SIZE > operator++( Array< ITEM_TYPE, SIZE > & items, int )
    {
        return { items };
    }

    /*
        EXAMPLES:

        #include "asn/core/array.hpp"


        void example_1d()
        {
            Array< const char *, 4 > array  { "Array", "Vector", "Queue", "Set" };

            for (auto string : array)
            {
                printf( "%s\n", string );
            }

            u32 index       = array ++.find_index( "Set" );
            array[ index ]  = "";
        }


        void example_2d()
        {
            Array< u8, 2, 4 > array_2d      { Array< u8, 4 > {1, 2, 3, 4}, {5, 6, 7, 8} };

            for (auto & array : array_2d)
            {
                for (auto element : array)
                {
                    printf( "%hu\n", element );
                }
            }

            for (auto & array : array_2d)
            {
                u32 index       = array ++.find_index( 4 );
                array[ index ]  = 0;
            }
        }
    */
}    // namespace AsnPlus

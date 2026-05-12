#pragma once

#include "etl/iterator.h"
#include "etl/multi_vector.h"

namespace AsnPlus
{
    template< typename ITEM_TYPE, size_t... SIZES >
    using Vector = etl::multi_vector< ITEM_TYPE, SIZES... >;

    template< typename ITEM_TYPE >
    using IVector = etl::ivector< ITEM_TYPE >;

    template< typename ITEM_TYPE >
    using VectorExt = etl::vector_ext< ITEM_TYPE >;

    // TODO: Rename this to something more serious
    template< typename ITEM_TYPE >
    class VectorExtUltimate : public VectorExt< ITEM_TYPE >
    {
        size_t & _size;

    public:
        VectorExtUltimate( ITEM_TYPE * buffer, size_t max_size, size_t & size ) :
            VectorExt< ITEM_TYPE >( buffer, max_size ),
            _size( size )
        {
            this->uninitialized_resize( _size );
        }

        void push_back( ITEM_TYPE item )
        {
            VectorExt< ITEM_TYPE >::push_back( item );
            _size++;
        }

        void erase( ITEM_TYPE * position )
        {
            VectorExt< ITEM_TYPE >::erase( position );
            _size--;
        }

        void clear()
        {
            VectorExt< ITEM_TYPE >::clear();
            _size = 0;
        }
    };

    template< typename ITEM_TYPE, size_t SIZE >
    class VectorUtil
    {
    public:
        Vector< ITEM_TYPE, SIZE > & items;

        size_t find_index( ITEM_TYPE & to_find )
        {
            uint32_t i = 0;

            for ( auto item : items )
            {
                if ( item == to_find ) break;
                i++;
            }
            return i;
        }

        void remove( size_t index ) { items.erase( items.begin() + index ); }
    };

    template< typename ITEM_TYPE, size_t SIZE >
    VectorUtil< ITEM_TYPE, SIZE > operator++( Vector< ITEM_TYPE, SIZE > & items, int )
    {
        return { items };
    }

    template< typename ITEM_TYPE, size_t MAX_LENGTH >
    struct VectorData
    {
        size_t    size = 0;
        ITEM_TYPE data[ MAX_LENGTH ];

        VectorExtUltimate< ITEM_TYPE > to_vector() { return VectorExtUltimate< ITEM_TYPE > { data, MAX_LENGTH, size }; }
    };
}    // namespace AsnPlus

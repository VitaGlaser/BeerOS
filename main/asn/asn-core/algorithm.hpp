#pragma once

#include "etl/algorithm.h"
#include "span.hpp"

namespace AsnPlus
{
    //  TODO: add missing constexpr modifiers (i.e. template<...> constexpr auto copy_if(...) {...})
    //  TODO: sort, add descriptions and examples
    template< typename T >
    struct Algorithm
    {
        Span< T > span;

        //  standard iterator algorithms
        //  ----------------------------------------------------------------

        template< typename ITERATOR >
        auto swap_ranges( ITERATOR other )
        {
            return etl::swap_ranges( span.begin(), span.end(), other );
        }

        template< typename FUNCTION >
        auto generate( FUNCTION function )
        {
            etl::generate( span.begin(), span.end(), function );
        }

        template< typename ITERATOR >
        auto copy( ITERATOR destination )
        {
            return etl::copy( span.begin(), span.end(), destination );
        }

        template< typename COUNT, typename ITERATOR >
        auto copy_n( COUNT count, ITERATOR destination )
        {
            return etl::copy_n( span.begin(), count, destination );
        }

        template< typename ITERATOR >
        auto copy_backward( ITERATOR destination )
        {
            return etl::copy_backward( span.begin(), span.end(), destination );
        }

        template< typename ITERATOR, typename UNARY_PREDICATE >
        auto copy_if( ITERATOR destination, UNARY_PREDICATE predicate )
        {
            return etl::copy_if( span.begin(), span.end(), destination, predicate );
        }

        template< typename ITERATOR >
        auto move( ITERATOR destination )
        {
            return etl::move( span.begin(), span.end(), destination );
        }

        template< typename ITERATOR >
        auto move_backward( ITERATOR destination )
        {
            return etl::move_backward( span.begin(), span.end(), destination );
        }

        auto reverse() { return etl::reverse( span.begin(), span.end() ); }

        template< typename ITERATOR >
        auto reverse_copy( ITERATOR destination )
        {
            return etl::reverse_copy( span.begin(), span.end(), destination );
        }

        template< typename VALUE, typename COMPARE >
        auto lower_bound( VALUE const & value, COMPARE compare )
        {
            return etl::lower_bound( span.begin(), span.end(), value, compare );
        }

        template< typename VALUE >
        auto lower_bound( VALUE const & value )
        {
            return etl::lower_bound( span.begin(), span.end(), value );
        }

        template< typename VALUE, typename COMPARE >
        auto upper_bound( VALUE const & value, COMPARE compare )
        {
            return etl::upper_bound( span.begin(), span.end(), value, compare );
        }

        template< typename VALUE >
        auto upper_bound( VALUE const & value )
        {
            return etl::upper_bound( span.begin(), span.end(), value );
        }

        template< typename VALUE, typename COMPARE >
        auto equal_range( VALUE const & value, COMPARE compare )
        {
            return etl::equal_range( span.begin(), span.end(), value, compare );
        }

        template< typename VALUE >
        auto equal_range( VALUE const & value )
        {
            return etl::equal_range( span.begin(), span.end(), value );
        }

        template< typename VALUE, typename COMPARE >
        auto binary_search( VALUE const & value, COMPARE compare )
        {
            return etl::binary_search( span.begin(), span.end(), value, compare );
        }

        template< typename VALUE >
        auto binary_search( VALUE const & value )
        {
            return etl::binary_search( span.begin(), span.end(), value );
        }

        template< typename VALUE >
        auto find( VALUE const & value )
        {
            return etl::find( span.begin(), span.end(), value );
        }

        template< typename PREDICATE >
        auto find_if( PREDICATE predicate )
        {
            return etl::find_if( span.begin(), span.end(), predicate );
        }

        template< typename PREDICATE >
        auto find_if_not( PREDICATE predicate )
        {
            return etl::find_if_not( span.begin(), span.end(), predicate );
        }

        template< typename VALUE >
        auto fill( VALUE const & value )
        {
            return etl::fill( span.begin(), span.end(), value );
        }

        template< typename COUNT, typename VALUE >
        auto fill_n( COUNT count, VALUE const & value )
        {
            return etl::fill_n( span.begin(), count, value );
        }

        template< typename VALUE >
        auto count( VALUE const & value )
        {
            return etl::count( span.begin(), span.end(), value );
        }

        template< typename VALUE, typename COMPARE >
        auto count_if( VALUE const & value, COMPARE compare )
        {
            return etl::count_if( span.begin(), span.end(), value, compare );
        }

        template< typename ITERATOR >
        auto equal( ITERATOR other )
        {
            return etl::equal( span.begin(), span.end(), other );
        }

        template< typename ITERATOR, typename COMPARE >
        auto equal( ITERATOR other, COMPARE compare )
        {
            return etl::equal( span.begin(), span.end(), other, compare );
        }

        template< typename CONTAINER >
        auto equal( CONTAINER const & other )
        {
            return etl::equal( span.begin(), span.end(), other.begin(), other.end() );
        }

        template< typename CONTAINER, typename COMPARE >
        auto equal( CONTAINER const & other, COMPARE compare )
        {
            return etl::equal( span.begin(), span.end(), other.begin(), other.end(), compare );
        }

        template< typename CONTAINER >
        auto lexicographical_compare( CONTAINER const & other )
        {
            return etl::lexicographical_compare( span.begin(), span.end(), other.begin(), other.end() );
        }

        template< typename CONTAINER, typename COMPARE >
        auto lexicographical_compare( CONTAINER const & other, COMPARE compare )
        {
            return etl::lexicographical_compare( span.begin(), span.end(), other.begin(), other.end(), compare );
        }

        template< typename UNARY_OPERATION >
        auto for_each( UNARY_OPERATION operation )
        {
            return etl::for_each( span.begin(), span.end(), operation );
        }

        template< typename ITERATOR, typename UNARY_OPERATION >
        auto transform( ITERATOR destination, UNARY_OPERATION operation )
        {
            return etl::transform( span.begin(), span.end(), destination, operation );
        }

        template< typename IN_ITERATOR, typename OUT_ITERATOR, typename BINARY_OPERATION >
        auto transform( IN_ITERATOR source2, OUT_ITERATOR destination, BINARY_OPERATION operation )
        {
            return etl::transform( span.begin(), span.end(), source2, destination, operation );
        }

        template< typename ITERATOR, typename VALUE >
        auto replace( VALUE const & old_value, VALUE const & new_value )
        {
            return etl::replace( span.begin(), span.end(), old_value, new_value );
        }

        template< typename ITERATOR, typename PREDICATE, typename VALUE >
        auto replace_if( PREDICATE predicate, VALUE const & new_value )
        {
            return etl::replace( span.begin(), span.end(), predicate, new_value );
        }

        template< typename CONTAINER >
        auto search( CONTAINER const & to_find )
        {
            return etl::search( span.begin(), span.end(), to_find.begin(), to_find.end() );
        }

        template< typename CONTAINER, typename COMPARE >
        auto search( CONTAINER const & to_find, COMPARE compare )
        {
            return etl::search( span.begin(), span.end(), to_find.begin(), to_find.end(), compare );
        }

        template< typename ITERATOR >
        auto rotate( ITERATOR middle )
        {
            return etl::rotate( span.begin(), span.end(), middle );
        }

        template< typename CONTAINER >
        auto find_end( CONTAINER const & to_find )
        {
            return etl::find_end( span.begin(), span.end(), to_find.begin(), to_find.end() );
        }

        template< typename CONTAINER, typename PREDICATE >
        auto find_end( CONTAINER const & to_find, PREDICATE predicate )
        {
            return etl::find_end( span.begin(), span.end(), to_find.begin(), to_find.end(), predicate );
        }

        auto min_element() { return etl::min_element( span.begin(), span.end() ); }

        template< typename PREDICATE >
        auto min_element( PREDICATE predicate )
        {
            return etl::min_element( span.begin(), span.end(), predicate );
        }

        auto max_element() { return etl::max_element( span.begin(), span.end() ); }

        template< typename PREDICATE >
        auto max_element( PREDICATE predicate )
        {
            return etl::max_element( span.begin(), span.end(), predicate );
        }

        auto minmax_element() { return etl::minmax_element( span.begin(), span.end() ); }

        template< typename PREDICATE >
        auto minmax_element( PREDICATE predicate )
        {
            return etl::minmax_element( span.begin(), span.end(), predicate );
        }

        bool is_sorted() { return etl::is_sorted( span.begin(), span.end() ); }

        template< typename PREDICATE >
        bool is_sorted( PREDICATE predicate )
        {
            return etl::is_sorted( span.begin(), span.end(), predicate );
        }

        bool is_sorted_until() { return etl::is_sorted_until( span.begin(), span.end() ); }

        template< typename PREDICATE >
        bool is_sorted_until( PREDICATE predicate )
        {
            return etl::is_sorted_until( span.begin(), span.end(), predicate );
        }

        template< typename ITERATOR, typename BINARY_PREDICATE >
        bool is_permutation( ITERATOR iterator, BINARY_PREDICATE predicate )
        {
            return etl::is_permutation( span.begin(), span.end(), iterator, predicate );
        }

        template< typename CONTAINER >
        bool is_permutation( CONTAINER const & container )
        {
            return etl::is_permutation( span.begin(), span.end(), container.begin(), container.end() );
        }

        template< typename CONTAINER, typename BINARY_PREDICATE >
        bool is_permutation( CONTAINER const & container, BINARY_PREDICATE predicate )
        {
            return etl::is_permutation( span.begin(), span.end(), container.begin(), container.end(), predicate );
        }

        template< typename UNARY_PREDICATE >
        bool is_partitioned( UNARY_PREDICATE predicate )
        {
            return etl::is_partitioned( span.begin(), span.end(), predicate );
        }

        template< typename UNARY_PREDICATE >
        auto partition_point( UNARY_PREDICATE predicate )
        {
            return etl::partition_point( span.begin(), span.end(), predicate );
        }

        template< typename ITERATOR_TRUE, typename ITERATOR_FALSE, typename PREDICATE >
        auto partition_copy( ITERATOR_TRUE destination_true, ITERATOR_FALSE destination_false, PREDICATE predicate )
        {
            return etl::partition_copy( span.begin(), span.end(), destination_true, destination_false, predicate );
        }

        template< typename UNARY_PREDICATE >
        auto constexpr all_of( UNARY_PREDICATE predicate )
        {
            return etl::all_of( span.begin(), span.end(), predicate );
        }

        template< typename UNARY_PREDICATE >
        auto constexpr any_of( UNARY_PREDICATE predicate )
        {
            return etl::any_of( span.begin(), span.end(), predicate );
        }

        template< typename UNARY_PREDICATE >
        auto constexpr none_of( UNARY_PREDICATE predicate )
        {
            return etl::none_of( span.begin(), span.end(), predicate );
        }

        void sort() { return etl::sort( span.begin(), span.end() ); }

        template< typename COMPARE >
        void sort( COMPARE compare )
        {
            return etl::sort( span.begin(), span.end(), compare );
        }

        void stable_sort() { return etl::stable_sort( span.begin(), span.end() ); }

        template< typename COMPARE >
        void stable_sort( COMPARE compare )
        {
            return etl::stable_sort( span.begin(), span.end(), compare );
        }

        template< typename VALUE >
        auto accumulate( VALUE sum )
        {
            return etl::accumulate( span.begin(), span.end(), sum );
        }

        template< typename VALUE, typename BINARY_OPERATION >
        auto accumalate( VALUE sum, BINARY_OPERATION operation )
        {
            return etl::accumulate( span.begin(), span.end(), sum, operation );
        }

        template< typename VALUE >
        auto remove( VALUE const & to_remove )
        {
            return etl::remove( span.begin(), span.end(), to_remove );
        }

        template< typename UNARY_PREDICATE >
        auto remove_if( UNARY_PREDICATE predicate )
        {
            return etl::remove_if( span.begin(), span.end(), predicate );
        }

        //  ETL-only iterator algorithms
        //  ----------------------------------------------------------------

        template< typename CONTAINER >
        auto copy_safe( CONTAINER const & destination )
        {
            return etl::copy_s( span.begin(), span.end(), destination.begin(), destination.end() );
        }

        template< typename COUNT, typename ITERATOR, typename COUNT2 >
        auto copy_safe_n( COUNT count, ITERATOR destination, COUNT2 count2 )
        {
            return etl::copy_n_s( span.begin(), count, destination, count2 );
        }

        template< typename CONTAINER, typename UNARY_PREDICATE >
        auto copy_safe_if( CONTAINER const & destination, UNARY_PREDICATE predicate )
        {
            return etl::copy_if_s( span.begin(), span.end(), destination.begin(), destination.end(), predicate );
        }

        template< typename COUNT, typename ITERATOR, typename UNARY_PREDICATE >
        auto copy_n_if( COUNT count, ITERATOR destination, UNARY_PREDICATE predicate )
        {
            return etl::copy_n_if( span.begin(), count, destination, predicate );
        }

        template< typename CONTAINER >
        auto move_safe( CONTAINER const & destination )
        {
            return etl::move_s( span.begin(), span.end(), destination.begin(), destination.end() );
        }

        template< typename VALUE >
        auto binary_find( VALUE const & to_find )
        {
            return etl::binary_find( span.begin(), span.end(), to_find );
        }

        template< typename VALUE, typename BINARY_PREDICATE, typename BINARY_EQUALITY >
        auto binary_find( VALUE const & to_find, BINARY_PREDICATE predicate, BINARY_EQUALITY equality )
        {
            return etl::binary_find( span.begin(), span.end(), to_find, predicate, equality );
        }

        template< typename UNARY_FUNCTION, typename UNARY_PREDICATE >
        auto for_each_if( UNARY_FUNCTION function, UNARY_PREDICATE predicate )
        {
            return etl::for_each_if( span.begin(), span.end(), function, predicate );
        }

        template< typename COUNT, typename UNARY_FUNCTION >
        auto for_each_n( COUNT count, UNARY_FUNCTION function )
        {
            return etl::for_each_n( span.begin(), count, function );
        }

        template< typename COUNT, typename UNARY_FUNCTION, typename UNARY_PREDICATE >
        auto for_each_if( COUNT count, UNARY_FUNCTION function, UNARY_PREDICATE predicate )
        {
            return etl::for_each_if( span.begin(), count, function, predicate );
        }

        template< typename CONTAINER, typename UNARY_OPERATION >
        auto transform_safe( CONTAINER const & destination, UNARY_OPERATION operation )
        {
            return etl::transform_s( span.begin(), span.end(), destination.begin(), destination.end(), operation );
        }

        template< typename COUNT, typename ITERATOR, typename UNARY_OPERATION >
        auto transform_n( COUNT count, ITERATOR destination, UNARY_OPERATION operation )
        {
            return etl::transform_n( span.begin(), span.end(), destination, operation );
        }

        template< typename ITERATOR, typename UNARY_OPERATION, typename UNARY_PREDICATE >
        auto transform_if( ITERATOR destination, UNARY_OPERATION operation, UNARY_PREDICATE predicate )
        {
            return etl::transform_if( span.begin(), span.end(), destination, operation );
        }

        template< typename IN_ITERATOR, typename OUT_ITERATOR, typename BINARY_OPERATION, typename BINARY_PREDICATE >
        auto transform_if(
            IN_ITERATOR      source2,
            OUT_ITERATOR     destination,
            BINARY_OPERATION operation,
            BINARY_PREDICATE predicate
        )
        {
            return etl::transform_if( span.begin(), span.end(), source2, destination, operation, predicate );
        }

        template< typename COUNT, typename ITERATOR, typename UNARY_OPERATION, typename UNARY_PREDICATE >
        auto transform_n_if( COUNT count, ITERATOR destination, UNARY_OPERATION operation, UNARY_PREDICATE predicate )
        {
            return etl::transform_n_if( span.begin(), count, destination, operation );
        }

        template<
            typename IN_ITERATOR,
            typename COUNT,
            typename OUT_ITERATOR,
            typename BINARY_OPERATION,
            typename BINARY_PREDICATE >
        auto transform_n_if(
            IN_ITERATOR      source2,
            COUNT            count,
            OUT_ITERATOR     destination,
            BINARY_OPERATION operation,
            BINARY_PREDICATE predicate
        )
        {
            return etl::transform_n_if( span.begin(), source2, count, destination, operation, predicate );
        }

        template<
            typename ITERATOR_TRUE,
            typename ITERATOR_FALSE,
            typename UNARY_OP_TRUE,
            typename UNARY_OP_FALSE,
            typename UNARY_PREDICATE >
        auto partition_transform(
            ITERATOR_TRUE   dest_true,
            ITERATOR_FALSE  dest_false,
            UNARY_OP_TRUE   op_true,
            UNARY_OP_FALSE  op_false,
            UNARY_PREDICATE predicate
        )
        {
            return etl::partition_transform(
                span.begin(), span.end(), dest_true, dest_false, op_true, op_false, predicate
            );
        }

        template<
            typename ITERATOR_IN,
            typename ITERATOR_TRUE,
            typename ITERATOR_FALSE,
            typename UNARY_OP_TRUE,
            typename UNARY_OP_FALSE,
            typename UNARY_PREDICATE >
        auto partition_transform(
            ITERATOR_IN     source2,
            ITERATOR_TRUE   dest_true,
            ITERATOR_FALSE  dest_false,
            UNARY_OP_TRUE   op_true,
            UNARY_OP_FALSE  op_false,
            UNARY_PREDICATE predicate
        )
        {
            return etl::partition_transform(
                span.begin(), span.end(), source2, dest_true, dest_false, op_true, op_false, predicate
            );
        }

        void shell_sort() { return etl::shell_sort( span.begin(), span.end() ); }

        template< typename COMPARE >
        void shell_sort( COMPARE compare )
        {
            return etl::shell_sort( span.begin(), span.end(), compare );
        }

        void insertion_sort() { return etl::insertion_sort( span.begin(), span.end() ); }

        template< typename COMPARE >
        void insertion_sort( COMPARE compare )
        {
            return etl::insertion_sort( span.begin(), span.end(), compare );
        }

        void selection_sort() { return etl::selection_sort( span.begin(), span.end() ); }

        template< typename COMPARE >
        void selection_sort( COMPARE compare )
        {
            return etl::selection_sort( span.begin(), span.end(), compare );
        }

        void heap_sort() { return etl::heap_sort( span.begin(), span.end() ); }

        template< typename COMPARE >
        void heap_sort( COMPARE compare )
        {
            return etl::heap_sort( span.begin(), span.end(), compare );
        }

        template< typename UNARY_PREDICATE >
        auto partition( UNARY_PREDICATE predicate )
        {
            return etl::partition( span.begin(), span.end(), predicate );
        }

        template< typename ITERATOR >
        void nth_element( ITERATOR nth_element )
        {
            return etl::nth_element( span.begin(), nth_element, span.end() );
        }

        //  heap functions
        //  ----------------------------------------------------------------

        void pop_heap() { return etl::pop_heap( span.begin(), span.end() ); }

        template< typename COMPARE >
        void pop_heap( COMPARE compare )
        {
            return etl::pop_heap( span.begin(), span.end(), compare );
        }

        void push_heap() { return etl::push_heap( span.begin(), span.end() ); }

        template< typename COMPARE >
        void push_heap( COMPARE compare )
        {
            return etl::push_heap( span.begin(), span.end(), compare );
        }

        void make_heap() { return etl::make_heap( span.begin(), span.end() ); }

        template< typename COMPARE >
        void make_heap( COMPARE compare )
        {
            return etl::make_heap( span.begin(), span.end(), compare );
        }

        bool is_heap() { return etl::is_heap( span.begin(), span.end() ); }

        template< typename COMPARE >
        bool is_heap( COMPARE compare )
        {
            return etl::is_heap( span.begin(), span.end(), compare );
        }

        void sort_heap() { return etl::sort_heap( span.begin(), span.end() ); }

        template< typename COMPARE >
        void sort_heap( COMPARE compare )
        {
            return etl::sort_heap( span.begin(), span.end(), compare );
        }
    };
}    // namespace AsnPlus

#pragma once

#include "split_iterator.hpp"
#include "string.hpp"
#include "to_string.hpp"

namespace AsnPlus
{
    struct StringUtil
    {
        IString & string;

        using const_pointer = IString::const_pointer;
        using size_type     = IString::size_type;
        using value_type    = IString::value_type;

        StringUtil( IString & string ) : string { string } {}

        operator IString &() { return string; }

        template< typename T >
        StringUtil & operator<<( const T & value )
        {
            append( value );
            return *this;
        }

        template< typename... TYPES >
        StringUtil append( TYPES &&... values )
        {
            ( serialize( std::forward< TYPES >( values ), string ), ... );
            return *this;
        }

        void trim_from_left( const_pointer trim_characters ) { etl::trim_from_left( string, trim_characters ); }

        void trim_whitespace_left() { etl::trim_whitespace_left( string ); }

        void trim_left( const_pointer delimiters ) { etl::trim_left( string, delimiters ); }

        void trim_from_right( const_pointer trim_characters ) { etl::trim_from_right( string, trim_characters ); }

        void trim_whitespace_right() { etl::trim_whitespace_right( string ); }

        void trim_right( const_pointer delimiters ) { etl::trim_right( string, delimiters ); }

        void trim_from( const_pointer trim_characters ) { etl::trim_from( string, trim_characters ); }

        void trim_whitespace() { etl::trim_whitespace( string ); }

        void trim( const_pointer delimiters ) { etl::trim( string, delimiters ); }

        void left_n( size_type n ) { etl::left_n( string, n ); }

        void right_n( size_type n ) { etl::right_n( string, n ); }

        void reverse() { etl::reverse( string ); }

        template< typename TPair >
        void replace_characters( const TPair * pairs_begin, const TPair * pairs_end )
        {
            return etl::replace_characters( string, pairs_begin, pairs_end );
        }

        template< typename TPair >
        void replace_strings( const TPair * pairs_begin, const TPair * pairs_end )
        {
            return etl::replace_strings( string, pairs_begin, pairs_end );
        }

        // template <typename TPointer>
        // auto find_first_of( size_t first, size_t last, TPointer delimiters )
        // {
        //     return etl::find_first_of( begin() + first, begin() + last, delimiters );
        // }

        // template <typename TPointer>
        // auto find_first_of( TPointer delimiters )
        // {
        //     return etl::find_first_of( string, delimiters );
        // }

        // template <typename TPointer>
        // auto find_first_not_of( size_t first, size_t last, TPointer delimiters )
        // {
        //     return etl::find_first_not_of( begin() + first, begin() + last, delimiters );
        // }

        // template <typename TPointer>
        // auto find_first_not_of( TPointer delimiters )
        // {
        //     return etl::find_first_not_of( string, delimiters );
        // }

        // template <typename TPointer>
        // auto find_last_of( size_t first, size_t last, TPointer delimiters )
        // {
        //     return etl::find_last_of( begin() + first, begin() + last, delimiters );
        // }

        // template <typename TPointer>
        // auto find_last_of( TPointer delimiters )
        // {
        //     return etl::find_last_of( string, delimiters );
        // }

        // template <typename TPointer>
        // auto find_last_not_of( size_t first, size_t last, TPointer delimiters )
        // {
        //     return etl::find_last_not_of( begin() + first, begin() + last, delimiters );
        // }

        // template <typename TPointer>
        // auto find_last_not_of( TPointer delimiters )
        // {
        //     return etl::find_last_not_of( string, delimiters );
        // }

        template< typename TStringView >
        auto get_token(
            const_pointer                        delimiters,
            const etl::optional< TStringView > & last_view,
            bool                                 ignore_empty_tokens
        )
        {
            return etl::get_token( string, delimiters, last_view, ignore_empty_tokens );
        }

        void pad_left( size_type required_size, value_type pad_char )
        {
            return etl::pad_left( string, required_size, pad_char );
        }

        void pad_right( size_type required_size, value_type pad_char )
        {
            return etl::pad_right( string, required_size, pad_char );
        }

        void pad( size_type required_size, etl::string_pad_direction pad_direction, value_type pad_char )
        {
            return etl::pad( string, required_size, pad_direction, pad_char );
        }

        void to_upper_case() { etl::to_upper_case( string ); }

        void to_lower_case() { etl::to_lower_case( string ); }

        void to_sentence_case() { etl::to_sentence_case( string ); }
    };

    template< size_t CAPACITY >
    class StringBuilder
    {
    public:
        String< CAPACITY > data;

        template< typename... TYPES >
        StringBuilder( TYPES... values )
        {
            ( serialize( values, data ), ... );
        }
    };

    class StringViewUtil
    {
    public:
        StringView view;

        SplitIterator split( char delimiter = ' ' ) { return { view, delimiter }; }

        Pair< StringView, StringView > split_at( u32 index )
        {
            return { view.substr( 0, index ), view.substr( index ) };
        }

        Pair< StringView, StringView > split_first_of( char delimiter = ' ', bool split_after_delimiter = false )
        {
            u32 i = 0;
            for ( auto ch : view )
            {
                if ( ch == delimiter ) break;
                i++;
            }

            return split_at( i + (u8) split_after_delimiter );
        }

        Pair< StringView, StringView > split_first_not_of( char delimiter = ' ', bool split_after_delimiter = false )
        {
            u32 i = 0;
            for ( auto ch : view )
            {
                if ( ch != delimiter ) break;
                i++;
            }

            return split_at( i + (u8) split_after_delimiter );
        }

        Pair< StringView, StringView > split_last_of( char delimiter = ' ', bool split_after_delimiter = false )
        {
            u32 i = 0;
            for ( i = view.size(); i > 0; i-- )
            {
                if ( view[ i - 1 ] == delimiter ) break;
            }

            return split_at( i - 1 + (u8) split_after_delimiter );
        }

        Pair< StringView, StringView > split_last_not_of( char delimiter = ' ', bool split_after_delimiter = false )
        {
            u32 i = 0;
            for ( i = view.size(); i > 0; i-- )
            {
                if ( view[ i - 1 ] != delimiter ) break;
            }

            return split_at( i - 1 + (u8) split_after_delimiter );
        }

        StringView first_delimited_string( char delimiter )
        {
            size_t start = view.find_first_of( delimiter );
            size_t end   = view.find_first_of( delimiter, start );

            return ( end == StringView::npos ) ? view.substr( start + 1, end - 1 ) : StringView {};
        }

        StringView find_first_of_prefix( char to_find )
        {
            size_t length = view.find_first_of( to_find );

            return view.substr( 0, length );
        }

        StringView find_last_of_prefix( char to_find )
        {
            size_t length = view.find_last_of( to_find );

            return view.substr( 0, length );
        }

        StringView find_first_of_suffix( char to_find )
        {
            size_t start = view.find_first_of( to_find );

            return view.substr( start + ( start != StringView::npos ) );
        }

        StringView find_last_of_suffix( char to_find )
        {
            size_t start = view.find_last_of( to_find );

            return view.substr( start + ( start != StringView::npos ) );
        }
    };

    inline StringUtil operator++( IString & string, int )
    {
        return string;
    }

    inline StringViewUtil operator++( StringView & view, int )
    {
        return { view };
    }
}    // namespace AsnPlus

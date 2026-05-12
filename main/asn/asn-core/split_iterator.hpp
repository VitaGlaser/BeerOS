#pragma once

#include "string.hpp"

namespace AsnPlus
{
    class SplitIterator
    {
    public:
        using pointer   = const StringView *;
        using reference = const StringView &;

        StringView remainder;
        StringView substring;
        char       delimiter;

        SplitIterator( const StringView & whole_string, char delimiter ) :
            remainder { whole_string },
            delimiter { delimiter }
        {
            initialize();
        }

        SplitIterator & operator++()
        {
            advance();
            return *this;
        }

        SplitIterator operator++( int )
        {
            auto result = *this;
            advance();
            return result;
        }

        reference operator*() const { return substring; }

        pointer operator->() const { return &substring; }

        bool operator==( const SplitIterator & other ) const
        {
            return ( remainder.size() == other.remainder.size() ) &&
                ( remainder.begin() == other.remainder.begin() || remainder.size() == 0 );
        }

        bool operator!=( const SplitIterator & other ) const { return ! ( *this == other ); }

        SplitIterator begin() const { return { remainder, delimiter }; }

        SplitIterator end() const { return { {}, delimiter }; }

        bool empty() const { return remainder.empty(); }

    protected:
        void initialize() { update_substring(); }

        void advance()
        {
            update_remainder();
            update_substring();
        }

        void update_substring()
        {
            auto slash_pos = remainder.find_first_of( delimiter );
            substring      = remainder.substr( 0, slash_pos );
        }

        void update_remainder() { remainder = remainder.substr( substring.size() + 1 ); }
    };
}    // namespace AsnPlus

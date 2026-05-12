#pragma once

#include "asn_module_config.hpp"

#include "at_parser_config.hpp"

#include "asn/asn-core/array.hpp"
#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/string.hpp"
#include "asn/asn-core/string_util.hpp"
#include "asn/asn-core/terminal_codes.hpp"
#include "asn/asn-core/vector.hpp"

#include "base.hpp"

#include <cstddef>
#include <cstdio>

namespace AsnPlus
{
    class UrcProcessor
    {
    public:
        using UrcCallback                                = Delegate< void( StringView ) >;

        UrcProcessor( const UrcProcessor & )             = delete;
        UrcProcessor & operator=( const UrcProcessor & ) = delete;

        UrcProcessor();

        bool registerUrcHandler( const StringView & prefix, UrcCallback callback );

        template< typename T, void ( T::*Method )( StringView ) >
        bool registerUrcHandler( const StringView & prefix, T & instance )
        {
            auto cb = UrcCallback::template create< T, Method >( instance );
            return registerUrcHandler( prefix, cb );
        }

        bool   unregisterUrcHandler( const StringView & prefix );
        size_t process( const ArrayView< uint8_t > & data );

    private:
        static constexpr char TAG[] = "UrcProcessor";
        using Log                   = AsnPlus::Logger< AtParserConfig::LOG_LEVEL, TAG >;

        struct Handler
        {
            etl::string< AtParserConfig::URC_PROCESSOR_MAX_PREFIX_SIZE > prefix;
            UrcCallback                                                  callback;
        };

        etl::vector< Handler, AtParserConfig::URC_PROCESSOR_MAX_HANDLERS_COUNT > _handlers;

        // TODO (DK): Remove this and use the hexdump from logger instead
        void _logBufferHex( ArrayView< const uint8_t > data );
        void _urcComplete( const StringView & urc );
    };
}    // namespace AsnPlus

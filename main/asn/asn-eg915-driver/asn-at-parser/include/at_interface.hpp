#pragma once

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/string.hpp"
#include "asn/asn-core/terminal_codes.hpp"
#include "asn/asn-core/vector.hpp"

#include "asn_module_config.hpp"
#include "at_uart.hpp"
#include "base.hpp"
#include "urc_processor.hpp"

#include <cstddef>
#include <cstdio>
#include <iomanip>

namespace AsnPlus
{
    class AtReader
    {
    public:
        AtReader( const AtReader & )             = delete;
        AtReader & operator=( const AtReader & ) = delete;

        using CompleteCallback                   = Delegate< void() >;

        AtReader() {}

        size_t process( const ArrayView< uint8_t > & data );
        void   startReadLine( IVector< uint8_t > & destination, CompleteCallback callback );
        void   startReadBytes( IVector< uint8_t > & destination, size_t length, CompleteCallback callback );

    private:
        static constexpr char TAG[] = "AtReader";
        using Log                   = AsnPlus::Logger< AtParserConfig::LOG_LEVEL, TAG >;

        enum class ReadingMode
        {
            None,
            CRLFLine,
            CountedBytes,
        };

        ReadingMode          _readingMode = ReadingMode::None;
        IVector< uint8_t > * _destination = nullptr;
        CompleteCallback     _finishCallback;
        String< 1024 >       _hexString;    // TODO (DK): remove this and use the hexdump from logger
        size_t               _rmCountedLength = 0;

        void   _logBufferHex( ArrayView< const uint8_t > data );
        size_t _processReadLine( const ArrayView< uint8_t > & data );
        size_t _processBytes( const ArrayView< uint8_t > & data );
    };

    class AtInterface
    {
    public:
        AtInterface( const AtInterface & )             = delete;
        AtInterface & operator=( const AtInterface & ) = delete;

        explicit AtInterface( AtUart & atuart );

        AtReader &     getReader();
        UrcProcessor & getUrcProcessor();

        void send( const uint8_t * data, size_t size, AtUart::SendCompleteDelegate sendCompleteCallback );
        void send( StringView data, AtUart::SendCompleteDelegate sendCompleteCallback );
        void readLine( IVector< uint8_t > & destination, AtReader::CompleteCallback callback );
        void readBytes( IVector< uint8_t > & destination, size_t length, AtReader::CompleteCallback callback );
        void sendDataMode( const uint8_t * data, size_t size );
        void sendDataMode( StringView data );

        template< typename Iterator >
        void send( Iterator start, Iterator end, AtUart::SendCompleteDelegate sendCompleteCallback )
        {
            Log::info( "Sending %u bytes (hex):", static_cast< unsigned >( std::distance( start, end ) ) );
            _logBufferHex( ArrayView< const uint8_t >( &( *start ), std::distance( start, end ) ) );
            _atUart.send( start, end, sendCompleteCallback );
        }

        template< typename Iterator >
        void sendDataMode( Iterator start, Iterator end )
        {
            Log::info( "Datamode send %d bytes", static_cast< int >( std::distance( start, end ) ) );
            _atUart.sendUnbuffered( start, end );
        }

    private:
        AtUart &     _atUart;
        UrcProcessor _urcProcessor;
        AtReader     _reader;

        static constexpr char TAG[] = "AtInterface";
        using Log                   = AsnPlus::Logger< AtParserConfig::LOG_LEVEL, TAG >;

        void _logBufferHex( ArrayView< const uint8_t > data );
    };
}    // namespace AsnPlus

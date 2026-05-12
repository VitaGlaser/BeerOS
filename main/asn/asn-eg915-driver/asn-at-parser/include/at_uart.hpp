#pragma once

#include "at_parser_config.hpp"

#include "asn/asn-core/array.hpp"
#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/vector.hpp"

#include "asn/asn-hal/include/peripherals/uart.hpp"

#include "base.hpp"

namespace AsnPlus
{
    class AtUart
    {
    public:
        AtUart( const AtUart & )             = delete;
        AtUart & operator=( const AtUart & ) = delete;

        explicit AtUart( IUart & uart ) : _uart( uart ) {}

        using UrcDelegate          = Delegate< size_t( const ArrayView< uint8_t > & ) >;
        using DataDelegate         = Delegate< size_t( const ArrayView< uint8_t > & ) >;
        using SendCompleteDelegate = Delegate< void() >;

        void setCallback( DataDelegate callback, UrcDelegate urcCallback );
        void poll();
        void send( const uint8_t * data, size_t size, SendCompleteDelegate sendCompleteCallback );
        void sendUnbuffered( const uint8_t * data, size_t size );
        void enterReadingMode();

        // TODO (DK): This should use the other send method to avoid code duplication
        template< typename Iterator >
        void send( Iterator start, Iterator end, SendCompleteDelegate sendCompleteCallback )
        {
            if ( _state != ProcessingState::Idle )
            {
                Log::error( "Cannot send. Interface in non idle state" );
                return;
            }
            _sendCompleteCallback = sendCompleteCallback;
            _sendBuffer.assign( start, end );
            _state = ProcessingState::SendPending;
            Log::hexdump( "Sending data", _sendBuffer.data(), _sendBuffer.size() );
            _uart.write( reinterpret_cast< const char * >( _sendBuffer.data() ), _sendBuffer.size() );
        }

        // TODO (DK): This should use the other send method to avoid code duplication
        template< typename Iterator >
        void sendUnbuffered( Iterator start, Iterator end )
        {
            _sendBuffer.assign( start, end );
            _uart.write( reinterpret_cast< const char * >( _sendBuffer.data() ), _sendBuffer.size() );
        }

    private:
        static constexpr char TAG[] = "AtUart";
        using Log                   = AsnPlus::Logger< AtParserConfig::LOG_LEVEL, TAG >;

        enum class ProcessingState
        {
            Idle,
            SendPending,
            Reading
        };
        IUart & _uart;

        etl::vector< uint8_t, AtParserConfig::AT_UART_RX_BUFFER_SIZE > _rxBuffer;
        etl::vector< uint8_t, AtParserConfig::AT_UART_TX_BUFFER_SIZE > _sendBuffer;

        DataDelegate         _dataCallback;
        UrcDelegate          _urcCallback;
        SendCompleteDelegate _sendCompleteCallback;

        ProcessingState _state = ProcessingState::Idle;

        void _process();
        bool _processIdle();
        bool _processSendPending();
        bool _processReading();
        bool _processURC();
    };
}    // namespace AsnPlus

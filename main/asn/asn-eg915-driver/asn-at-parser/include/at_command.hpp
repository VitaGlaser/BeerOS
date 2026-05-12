#pragma once

#include "asn/asn-core/array.hpp"
#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/string.hpp"

#include "asn/asn-core/vector.hpp"

#include "at_interface.hpp"
#include "command_builder.hpp"

#include <chrono>

namespace AsnPlus
{
    using CommandFinishedCallback = Delegate< void( bool ) >;

    class ATCommand
    {
    public:
        ATCommand( AtInterface & interface, IVector< uint8_t > & dataBuffer, StringView commandBase ) :
            _interface( interface ),
            _dataBuffer( dataBuffer ),
            _commandBase( commandBase ),
            _isFinished( false ),
            _result( false ),
            _requiresRebuild( true )
        {
        }

        CommandBuilder & builder();
        bool             isFinished();
        bool             isOk();

        void send( CommandFinishedCallback onFinished = CommandFinishedCallback() );
        bool sendAsync( std::chrono::milliseconds timeout = std::chrono::milliseconds( 1000 ) );

    protected:
        AtInterface &        _interface;
        IVector< uint8_t > & _dataBuffer;
        StringView           _commandBase;
        CommandBuilder       _builder { _dataBuffer, _requiresRebuild, _commandBase };

        virtual void commandStarted() {}

        virtual void sendComplete();
        void         expectOk();
        void         finish( bool result );

        bool isLineCommandEnding( ArrayView< uint8_t > view, bool & isOk );

    private:
        static constexpr char TAG[] = "AtCommand";
        using Log                   = AsnPlus::Logger< AtParserConfig::LOG_LEVEL, TAG >;

        bool                    _isFinished;
        bool                    _result;
        bool                    _requiresRebuild;
        CommandFinishedCallback _onFinished;

        void _onSendFinished();
        void _expectOkReadComplete();
    };
}    // namespace AsnPlus

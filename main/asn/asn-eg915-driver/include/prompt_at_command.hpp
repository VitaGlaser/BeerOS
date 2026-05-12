#pragma once

#include "asn-at-parser/include/at_command.hpp"
#include "asn-at-parser/include/at_interface.hpp"

#include "asn/asn-core/array.hpp"
#include "asn/asn-core/string.hpp"
#include "asn/asn-core/string_util.hpp"

namespace AsnPlus
{
    class PromptAtCommand : public ATCommand
    {
    public:
        PromptAtCommand(
            AtInterface &        interface,
            IVector< uint8_t > & dataBuffer,
            StringView           commandBase,
            bool                 expectLineAfterSend,
            StringView           prompt       = ">",
            bool                 promptNoCrlf = false
        ) :
            ATCommand( interface, dataBuffer, commandBase ),
            _expectLineAfterSend( expectLineAfterSend ),
            _prompt( prompt ),
            _promptNoCrlf( promptNoCrlf )
        {
        }

        void setData( ArrayView< const uint8_t > data ) { _data = data; }

    protected:
        void commandStarted() override {}

        void sendComplete() override;

    private:
        ArrayView< const uint8_t > _data;
        bool                        _expectLineAfterSend;
        StringView                  _prompt;
        bool                        _promptNoCrlf;

        void _promptReceived();
        void _onPayloadSent();
        void _onFinalResponse();

        void _sendOfDataComplete() {}

        void _sendResponseReceived() { expectOk(); }
    };
}    // namespace AsnPlus

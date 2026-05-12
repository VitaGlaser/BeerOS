#pragma once

#include "asn-at-parser/include/at_command.hpp"
#include "asn-at-parser/include/at_interface.hpp"

#include "asn/asn-core/array.hpp"
#include "asn/asn-core/delegate.hpp"

namespace AsnPlus
{
    class ReadAtCommand : public ATCommand
    {
    public:
        using IntermediateLineCallback = Delegate< void( ArrayView< uint8_t > ) >;

        ReadAtCommand(
            AtInterface &        interface,
            IVector< uint8_t > & dataBuffer,
            etl::string_view     commandBase
        ) :
            ATCommand( interface, dataBuffer, commandBase )
        {
        }

        void setIntermediateLineCallback( IntermediateLineCallback cb ) { _intermediateCallback = cb; }

    protected:
        void sendComplete() override;

    private:
        IntermediateLineCallback _intermediateCallback;

        void _readNextLine();
        void _onLineReceived();
    };
}    // namespace AsnPlus

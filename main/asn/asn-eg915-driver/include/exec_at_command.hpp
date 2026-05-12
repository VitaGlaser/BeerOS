#pragma once

/**
 * @brief AT command wrapper for execution-form commands that return intermediate response lines.
 *
 * Use this for commands invoked without a suffix (no `?` or `=`), such as `AT+QIGETERROR`,
 * that return one or more `+PREFIX: data` lines before the final OK/ERROR.
 *
 * For read-form commands (AT+CMD?) use ReadAtCommand instead.
 */

#include "asn-at-parser/include/at_command.hpp"
#include "asn-at-parser/include/at_interface.hpp"

#include "asn/asn-core/array.hpp"
#include "asn/asn-core/delegate.hpp"

namespace AsnPlus
{
    class ExecAtCommand : public ATCommand
    {
    public:
        using IntermediateLineCallback = Delegate< void( ArrayView< uint8_t > ) >;

        ExecAtCommand(
            AtInterface &        interface,
            IVector< uint8_t > & dataBuffer,
            StringView           commandBase
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

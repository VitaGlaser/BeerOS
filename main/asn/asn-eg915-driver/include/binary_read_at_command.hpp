#pragma once

#include "asn-at-parser/include/at_command.hpp"
#include "asn-at-parser/include/at_interface.hpp"

#include "asn/asn-core/array.hpp"
#include "asn/asn-core/delegate.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/string.hpp"
#include "asn/asn-core/string_util.hpp"
#include "asn/asn-core/vector.hpp"

namespace AsnPlus
{
    class BinaryReadAtCommand : public ATCommand
    {
    public:
        using ParseDataReadLengthCallback = Delegate< bool( ArrayView< uint8_t >, size_t &, size_t & ) >;

        BinaryReadAtCommand(
            AtInterface &               interface,
            IVector< uint8_t > &        dataBuffer,
            StringView                  commandBase,
            ParseDataReadLengthCallback cb
        );

        size_t getRealReadLength() const { return _realReadLength; }

        size_t getRemainingDataLength() const { return _remainingDataLength; }

        void setDestinationBuffer( IVector< uint8_t > & data ) { _responseBuffer = &data; }

    protected:
        void commandStarted() override {}

        void sendComplete() override;

    private:
        static constexpr char TAG[] = "BinaryReadAtCommand";
        using Log                   = Logger< 2, TAG >;

        ParseDataReadLengthCallback _parseCb;
        size_t                      _realReadLength      = 0;
        size_t                      _remainingDataLength = 0;
        IVector< uint8_t > *        _responseBuffer      = nullptr;

        void _lengthHeaderReceived();
        void _dataReadFinished();
    };
}    // namespace AsnPlus

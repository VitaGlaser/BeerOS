#pragma once

#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/types.hpp"

namespace AsnPlus::DataSource
{
    class Base
    {
    public:
        Base()                    = default;
        virtual bool initialize() = 0;

        virtual void poll()       = 0;

        uint32_t read() const { return _value; }

        uint64_t getId() const { return _id; }

        void setId( uint64_t id ) { _id = id; }

    protected:
        uint64_t _id    = 0;
        uint32_t _value = 0;
    };
}    // namespace AsnPlus::DataSource

#pragma once

#include "etl/string.h"
#include "etl/string_stream.h"
#include "etl/string_utilities.h"
#include "pair.hpp"
#include "result.hpp"
#include "types.hpp"

namespace AsnPlus
{
    template< size_t LENGTH >
    using String       = etl::string< LENGTH >;
    using IString      = etl::istring;
    using StringView   = etl::string_view;
    using StringStream = etl::string_stream;
    using Format       = etl::format_spec;
    using StringExt    = etl::string_ext;

    template< size_t LENGTH >
    struct StringData
    {
        char data[ LENGTH ];

        StringExt to_string()
        {
            terminate();
            return StringExt { data, data, LENGTH };
        }

        void terminate() { data[ LENGTH - 1 ] = 0; }
    };
}    // namespace AsnPlus

#pragma once

#include "etl/base64_decoder.h"

namespace AsnPlus
{
    namespace Base64
    {
        using IDecoder = etl::ibase64_decoder;

        template< size_t BUFFER_SIZE >
        using Rfc2152Decoder = etl::base64_rfc2152_decoder< BUFFER_SIZE >;

        template< size_t BUFFER_SIZE >
        using Rfc3501Decoder = etl::base64_rfc3501_decoder< BUFFER_SIZE >;

        template< size_t BUFFER_SIZE >
        using Rfc4648Decoder = etl::base64_rfc4648_decoder< BUFFER_SIZE >;

        template< size_t BUFFER_SIZE >
        using Rfc4648PaddingDecoder = etl::base64_rfc4648_padding_decoder< BUFFER_SIZE >;

        template< size_t BUFFER_SIZE >
        using Rfc4648UrlDecoder = etl::base64_rfc4648_url_decoder< BUFFER_SIZE >;

        template< size_t BUFFER_SIZE >
        using Rfc4648UrlPaddingDecoder = etl::base64_rfc4648_url_padding_decoder< BUFFER_SIZE >;
    }    // namespace Base64
}    // namespace AsnPlus

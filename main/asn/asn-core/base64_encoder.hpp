#pragma once

#include "etl/base64_encoder.h"

namespace AsnPlus
{
    namespace Base64
    {
        using IEncoder = etl::ibase64_encoder;

        template< size_t BUFFER_SIZE >
        using Rfc2152Encoder = etl::base64_rfc2152_encoder< BUFFER_SIZE >;

        template< size_t BUFFER_SIZE >
        using Rfc3501Encoder = etl::base64_rfc3501_encoder< BUFFER_SIZE >;

        template< size_t BUFFER_SIZE >
        using Rfc4648Encoder = etl::base64_rfc4648_encoder< BUFFER_SIZE >;

        template< size_t BUFFER_SIZE >
        using Rfc4648PaddingEncoder = etl::base64_rfc4648_padding_encoder< BUFFER_SIZE >;

        template< size_t BUFFER_SIZE >
        using Rfc4648UrlEncoder = etl::base64_rfc4648_url_encoder< BUFFER_SIZE >;

        template< size_t BUFFER_SIZE >
        using Rfc4648UrlPaddingEncoder = etl::base64_rfc4648_url_padding_encoder< BUFFER_SIZE >;
    }    // namespace Base64
}    // namespace AsnPlus

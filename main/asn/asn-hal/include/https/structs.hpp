#pragma once

#include "asn/asn-core/types.hpp"
#include "asn/asn-core/vector.hpp"

namespace AsnPlus::Https
{
    enum class Method
    {
        GET,
        POST
    };

    struct HeaderKeyValue
    {
        const char * key;
        const char * value;
    };

    struct Request
    {
        Method                            method;
        IVector< HeaderKeyValue > * headers = nullptr;
        IVector< uint8_t > *        payload = nullptr;
    };

    struct Response
    {
        static constexpr uint8_t          MAX_CONTENT_TYPE_LENGTH = 31;
        uint16_t                          status                  = 400;
        IVector< HeaderKeyValue > * headers                 = nullptr;
        IVector< uint8_t > *        response                = nullptr;
    };

    static constexpr uint8_t MAX_URL_LENGTH          = 255;
    static constexpr uint8_t MAX_HEADERS             = 16;
    static constexpr uint8_t MAX_HEADER_VALUE_LENGTH = 255;

    // TODO: Add enum for those
    static constexpr HeaderKeyValue COMMON_HEADERS[] = {
        {"Content-Type",    nullptr},
        {"Content-Length",  nullptr},
        {"Authorization",   nullptr},
        {"User-Agent",      nullptr},
        {"Accept",          nullptr},
        {"Accept-Encoding", nullptr},
        {"Connection",      nullptr},
        {"Host",            nullptr},
    };
}    // namespace AsnPlus::Https
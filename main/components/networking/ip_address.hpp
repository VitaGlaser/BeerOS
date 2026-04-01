#pragma once

#include "asn/asn-core/etl/string.h"
#include <cstdio>
#include <cstdint>

namespace AsnPlus::Networking
{
    // IP Address structure for type-safe network configuration
    struct IPAddress
    {
        uint8_t octets[4];
        
        constexpr IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d) 
            : octets{a, b, c, d} {}
        
        // Default constructor
        constexpr IPAddress() : octets{0, 0, 0, 0} {}

        constexpr IPAddress(uint32_t ip) : octets{ static_cast<uint8_t>(ip & 0xFF),
                                                  static_cast<uint8_t>((ip >> 8) & 0xFF),
                                                  static_cast<uint8_t>((ip >> 16) & 0xFF),
                                                  static_cast<uint8_t>((ip >> 24) & 0xFF) }{}
        
        // Convert to string for logging and API calls
        void toString(char* buffer, size_t size) const
        {
            snprintf(buffer, size, "%d.%d.%d.%d", octets[0], octets[1], octets[2], octets[3]);
        }
        
        // Get as string for ETL string conversion
        etl::string<16> toString() const
        {
            char buffer[16];
            toString(buffer, sizeof(buffer));
            return etl::string<16>(buffer);
        }
        
        // Convert to network byte order for lwip
        uint32_t toNetworkOrder() const
        {
            return (static_cast<uint32_t>(octets[0]) << 24) |
                   (static_cast<uint32_t>(octets[1]) << 16) |
                   (static_cast<uint32_t>(octets[2]) << 8) |
                   static_cast<uint32_t>(octets[3]);
        }
        
        // Convert to uint32_t in little-endian format (for ESP-IDF esp_netif)
        uint32_t toUint32() const
        {
            return static_cast<uint32_t>(octets[0]) |
                   (static_cast<uint32_t>(octets[1]) << 8) |
                   (static_cast<uint32_t>(octets[2]) << 16) |
                   (static_cast<uint32_t>(octets[3]) << 24);
        }
        
        void toArrayInNetworkOrder(uint8_t *arr)
        {

            arr[0] = octets[0];
            arr[1] = octets[1];
            arr[2] = octets[2];
            arr[3] = octets[3];
        }
        
        // Parse from string
        static IPAddress fromString(const char* ipStr)
        {
            uint8_t a, b, c, d;
            sscanf(ipStr, "%hhu.%hhu.%hhu.%hhu", &a, &b, &c, &d);
            return IPAddress(a, b, c, d);
        }
        
        // Equality comparison
        bool operator==(const IPAddress& other) const
        {
            return octets[0] == other.octets[0] &&
                   octets[1] == other.octets[1] &&
                   octets[2] == other.octets[2] &&
                   octets[3] == other.octets[3];
        }
        
        // Inequality comparison
        bool operator!=(const IPAddress& other) const
        {
            return !(*this == other);
        }
        
        // Check if IP is valid (not 0.0.0.0)
        bool isValid() const
        {
            return !(octets[0] == 0 && octets[1] == 0 && octets[2] == 0 && octets[3] == 0);
        }
        
        // Check if IP is local/private
        bool isPrivate() const
        {
            // 10.0.0.0/8
            if (octets[0] == 10) return true;
            // 172.16.0.0/12
            if (octets[0] == 172 && octets[1] >= 16 && octets[1] <= 31) return true;
            // 192.168.0.0/16
            if (octets[0] == 192 && octets[1] == 168) return true;
            return false;
        }


        static constexpr IPAddress empty()
        {
            return IPAddress(0, 0, 0, 0);
        }
    };
}    // namespace AsnPlus::Networking

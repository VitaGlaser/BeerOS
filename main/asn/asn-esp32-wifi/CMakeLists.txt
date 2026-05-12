cmake_minimum_required(VERSION 3.16)

add_library(asn_esp32_wifi STATIC IMPORTED GLOBAL)
add_library(AsnPlus::Esp32Wifi ALIAS asn_esp32_wifi)

set_target_properties(asn_esp32_wifi PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libasn_esp32_wifi.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include;${CMAKE_CURRENT_LIST_DIR}/../.."
    INTERFACE_LINK_LIBRARIES "AsnPlus::Hal;idf::esp_wifi;idf::esp_netif;idf::esp_event;idf::esp_eth;idf::esp_http_client;idf::esp_https_server;idf::esp-tls;idf::mbedtls;idf::esp_coex;idf::lwip;idf::driver;idf::mqtt"
    INTERFACE_COMPILE_FEATURES cxx_std_20
)

cmake_minimum_required(VERSION 3.16)

add_library(asn_esp32_hal STATIC IMPORTED GLOBAL)
add_library(AsnPlus::Esp32Hal ALIAS asn_esp32_hal)

set_target_properties(asn_esp32_hal PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libasn_esp32_hal.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include;${CMAKE_CURRENT_LIST_DIR}/../.."
    INTERFACE_LINK_LIBRARIES "AsnPlus::Hal;idf::driver;idf::esp_adc;idf::nvs_flash"
    INTERFACE_COMPILE_FEATURES cxx_std_20
)

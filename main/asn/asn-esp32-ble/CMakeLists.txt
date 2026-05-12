cmake_minimum_required(VERSION 3.16)

add_library(asn_esp32_ble STATIC IMPORTED GLOBAL)
add_library(AsnPlus::Esp32Ble ALIAS asn_esp32_ble)

set_target_properties(asn_esp32_ble PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libasn_esp32_ble.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include;${CMAKE_CURRENT_LIST_DIR}/../.."
    INTERFACE_LINK_LIBRARIES "AsnPlus::Hal;idf::bt;idf::app_update;idf::nvs_flash"
    INTERFACE_COMPILE_FEATURES cxx_std_20
)

cmake_minimum_required(VERSION 3.16)

add_library(asn_esp32_modbus STATIC IMPORTED GLOBAL)
add_library(AsnPlus::Esp32Modbus ALIAS asn_esp32_modbus)

set_target_properties(asn_esp32_modbus PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libasn_esp32_modbus.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include;${CMAKE_CURRENT_LIST_DIR}/../.."
    INTERFACE_LINK_LIBRARIES "AsnPlus::Core;idf::driver;idf::freertos;idf::esp_timer;idf::espressif__esp-modbus"
    INTERFACE_COMPILE_FEATURES cxx_std_20
)

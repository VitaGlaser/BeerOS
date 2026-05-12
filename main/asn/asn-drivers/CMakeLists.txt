cmake_minimum_required(VERSION 3.16)

add_library(asn_drivers STATIC IMPORTED GLOBAL)
add_library(AsnPlus::Drivers ALIAS asn_drivers)

set_target_properties(asn_drivers PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libasn_drivers.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include;${CMAKE_CURRENT_LIST_DIR}/../.."
    INTERFACE_LINK_LIBRARIES "AsnPlus::Core;AsnPlus::Hal;AsnPlus::Esp32Modbus"
    INTERFACE_COMPILE_FEATURES cxx_std_23
)

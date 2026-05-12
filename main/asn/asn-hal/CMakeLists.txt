cmake_minimum_required(VERSION 3.16)

add_library(asn_hal STATIC IMPORTED GLOBAL)
add_library(AsnPlus::Hal ALIAS asn_hal)

set_target_properties(asn_hal PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libasn_hal.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include;${CMAKE_CURRENT_LIST_DIR}/../.."
    INTERFACE_LINK_LIBRARIES "AsnPlus::Core"
    INTERFACE_COMPILE_FEATURES cxx_std_20
)

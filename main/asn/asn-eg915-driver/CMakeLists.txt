cmake_minimum_required(VERSION 3.16)

add_subdirectory(asn-at-parser)

add_library(asn_eg915_driver STATIC IMPORTED GLOBAL)
add_library(AsnPlus::Eg915Driver ALIAS asn_eg915_driver)

set_target_properties(asn_eg915_driver PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libasn_eg915_driver.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include;${CMAKE_CURRENT_LIST_DIR};${CMAKE_CURRENT_LIST_DIR}/../.."
    INTERFACE_LINK_LIBRARIES "AsnPlus::AtParser"
    INTERFACE_COMPILE_FEATURES cxx_std_23
)

cmake_minimum_required(VERSION 3.16)

add_library(asn_at_parser STATIC IMPORTED GLOBAL)
add_library(AsnPlus::AtParser ALIAS asn_at_parser)

set_target_properties(asn_at_parser PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libasn_at_parser.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include;${CMAKE_CURRENT_LIST_DIR}/..;${CMAKE_CURRENT_LIST_DIR}/../../.."
    INTERFACE_LINK_LIBRARIES "AsnPlus::Core;AsnPlus::Hal"
    INTERFACE_COMPILE_FEATURES cxx_std_23
)

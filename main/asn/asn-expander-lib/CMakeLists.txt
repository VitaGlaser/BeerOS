cmake_minimum_required(VERSION 3.16)

add_library(asn_expander_data STATIC IMPORTED GLOBAL)
set_target_properties(asn_expander_data PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libasn_expander_data.a"
)

add_library(asn_expander_lib STATIC IMPORTED GLOBAL)
add_library(AsnPlus::ExpanderLib ALIAS asn_expander_lib)

set_target_properties(asn_expander_lib PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libasn_expander_lib.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include;${CMAKE_CURRENT_LIST_DIR}/../.."
    INTERFACE_LINK_LIBRARIES "asn_expander_data;AsnPlus::Core;idf::driver"
    INTERFACE_COMPILE_FEATURES cxx_std_23
)

cmake_minimum_required(VERSION 3.16)

add_library(asn_core INTERFACE)
add_library(AsnPlus::Core ALIAS asn_core)

target_compile_features(asn_core
    INTERFACE
        cxx_std_20
)

target_include_directories(asn_core
    INTERFACE
        "${CMAKE_CURRENT_LIST_DIR}/../.."
)

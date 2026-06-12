idf_component_register(
    SRCS
        "main.cpp"
    INCLUDE_DIRS
        "."
        "${CMAKE_CURRENT_BINARY_DIR}"
    REQUIRES
        esp_https_ota
        esp_http_client
        app_update
)

message(STATUS "Generating build_info.h")

string(TIMESTAMP BUILD_TIME "%Y-%m-%d %H:%M:%S" UTC)
string(TIMESTAMP BUILD_UNIX_SECS "%s" UTC)
math(EXPR BUILD_TIME_MS "${BUILD_UNIX_SECS} * 1000")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/build_info.h"
"#pragma once\n"
"#define BUILD_TIMESTAMP \"${BUILD_TIME}\"\n"
"#define BUILD_TIMESTAMP_MS ${BUILD_TIME_MS}\n"
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-core)
    add_subdirectory(asn/asn-core)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-hal)
    add_subdirectory(asn/asn-hal)
    target_link_libraries(${COMPONENT_LIB} PRIVATE AsnPlus::Hal)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-esp32-hal)
    add_subdirectory(asn/asn-esp32-hal)
    target_link_libraries(${COMPONENT_LIB} PRIVATE AsnPlus::Esp32Hal)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-esp32-ble)
    add_subdirectory(asn/asn-esp32-ble)
    target_link_libraries(${COMPONENT_LIB} PRIVATE AsnPlus::Esp32Ble)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-esp32-wifi)
    add_subdirectory(asn/asn-esp32-wifi)
    target_link_libraries(${COMPONENT_LIB} PRIVATE AsnPlus::Esp32Wifi)
    target_link_libraries(${COMPONENT_LIB} PRIVATE idf::esp_http_server)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-esp32-modbus)
    add_subdirectory(asn/asn-esp32-modbus)
    target_link_libraries(${COMPONENT_LIB} PRIVATE AsnPlus::Esp32Modbus)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-eg915-driver)
    add_subdirectory(asn/asn-eg915-driver)
    target_link_libraries(${COMPONENT_LIB} PRIVATE AsnPlus::Eg915Driver)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-expander-lib)
    add_subdirectory(asn/asn-expander-lib)
    target_link_libraries(${COMPONENT_LIB} PRIVATE AsnPlus::ExpanderLib)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-drivers)
    add_subdirectory(asn/asn-drivers)
    target_link_libraries(${COMPONENT_LIB} PRIVATE AsnPlus::Drivers)
endif()

include(GNUInstallDirs)

set(ASN_CUSTOMER_EXPORT_COMPONENT customer-export)
set(ASN_EXPORT_CMAKE_DIR "${CMAKE_CURRENT_BINARY_DIR}/customer_export_cmake")

file(MAKE_DIRECTORY "${ASN_EXPORT_CMAKE_DIR}")

file(WRITE "${ASN_EXPORT_CMAKE_DIR}/asn-core-CMakeLists.txt"
[=[
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
]=]
)

file(WRITE "${ASN_EXPORT_CMAKE_DIR}/asn-hal-CMakeLists.txt"
[=[
cmake_minimum_required(VERSION 3.16)

add_library(asn_hal STATIC IMPORTED GLOBAL)
add_library(AsnPlus::Hal ALIAS asn_hal)

set_target_properties(asn_hal PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libasn_hal.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include;${CMAKE_CURRENT_LIST_DIR}/../.."
    INTERFACE_LINK_LIBRARIES "AsnPlus::Core"
    INTERFACE_COMPILE_FEATURES cxx_std_20
)
]=]
)

file(WRITE "${ASN_EXPORT_CMAKE_DIR}/asn-esp32-hal-CMakeLists.txt"
[=[
cmake_minimum_required(VERSION 3.16)

add_library(asn_esp32_hal STATIC IMPORTED GLOBAL)
add_library(AsnPlus::Esp32Hal ALIAS asn_esp32_hal)

set_target_properties(asn_esp32_hal PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libasn_esp32_hal.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include;${CMAKE_CURRENT_LIST_DIR}/../.."
    INTERFACE_LINK_LIBRARIES "AsnPlus::Hal;idf::driver;idf::esp_adc;idf::nvs_flash"
    INTERFACE_COMPILE_FEATURES cxx_std_20
)
]=]
)

file(WRITE "${ASN_EXPORT_CMAKE_DIR}/asn-esp32-ble-CMakeLists.txt"
[=[
cmake_minimum_required(VERSION 3.16)

add_library(asn_esp32_ble STATIC IMPORTED GLOBAL)
add_library(AsnPlus::Esp32Ble ALIAS asn_esp32_ble)

set_target_properties(asn_esp32_ble PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libasn_esp32_ble.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include;${CMAKE_CURRENT_LIST_DIR}/../.."
    INTERFACE_LINK_LIBRARIES "AsnPlus::Hal;idf::bt;idf::app_update;idf::nvs_flash"
    INTERFACE_COMPILE_FEATURES cxx_std_20
)
]=]
)

file(WRITE "${ASN_EXPORT_CMAKE_DIR}/asn-esp32-modbus-CMakeLists.txt"
[=[
cmake_minimum_required(VERSION 3.16)

add_library(asn_esp32_modbus STATIC IMPORTED GLOBAL)
add_library(AsnPlus::Esp32Modbus ALIAS asn_esp32_modbus)

set_target_properties(asn_esp32_modbus PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libasn_esp32_modbus.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include;${CMAKE_CURRENT_LIST_DIR}/../.."
    INTERFACE_LINK_LIBRARIES "AsnPlus::Core;idf::driver;idf::freertos;idf::esp_timer;idf::espressif__esp-modbus"
    INTERFACE_COMPILE_FEATURES cxx_std_20
)
]=]
)

file(WRITE "${ASN_EXPORT_CMAKE_DIR}/asn-esp32-wifi-CMakeLists.txt"
[=[
cmake_minimum_required(VERSION 3.16)

add_library(asn_esp32_wifi STATIC IMPORTED GLOBAL)
add_library(AsnPlus::Esp32Wifi ALIAS asn_esp32_wifi)

set_target_properties(asn_esp32_wifi PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libasn_esp32_wifi.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include;${CMAKE_CURRENT_LIST_DIR}/../.."
    INTERFACE_LINK_LIBRARIES "AsnPlus::Hal;idf::esp_wifi;idf::esp_netif;idf::esp_event;idf::esp_eth;idf::esp_http_client;idf::esp_https_server;idf::esp-tls;idf::mbedtls;idf::esp_coex;idf::lwip;idf::driver;idf::mqtt"
    INTERFACE_COMPILE_FEATURES cxx_std_20
)
]=]
)

file(WRITE "${ASN_EXPORT_CMAKE_DIR}/asn-eg915-driver-CMakeLists.txt"
[=[
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
]=]
)

file(WRITE "${ASN_EXPORT_CMAKE_DIR}/asn-eg915-driver-asn-at-parser-CMakeLists.txt"
[=[
cmake_minimum_required(VERSION 3.16)

add_library(asn_at_parser STATIC IMPORTED GLOBAL)
add_library(AsnPlus::AtParser ALIAS asn_at_parser)

set_target_properties(asn_at_parser PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libasn_at_parser.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include;${CMAKE_CURRENT_LIST_DIR}/..;${CMAKE_CURRENT_LIST_DIR}/../../.."
    INTERFACE_LINK_LIBRARIES "AsnPlus::Core;AsnPlus::Hal"
    INTERFACE_COMPILE_FEATURES cxx_std_23
)
]=]
)

file(WRITE "${ASN_EXPORT_CMAKE_DIR}/asn-drivers-CMakeLists.txt"
[=[
cmake_minimum_required(VERSION 3.16)

add_library(asn_drivers STATIC IMPORTED GLOBAL)
add_library(AsnPlus::Drivers ALIAS asn_drivers)

set_target_properties(asn_drivers PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libasn_drivers.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include;${CMAKE_CURRENT_LIST_DIR}/../.."
    INTERFACE_LINK_LIBRARIES "AsnPlus::Core;AsnPlus::Hal;AsnPlus::Esp32Modbus"
    INTERFACE_COMPILE_FEATURES cxx_std_23
)
]=]
)

file(WRITE "${ASN_EXPORT_CMAKE_DIR}/asn-expander-lib-CMakeLists.txt"
[=[
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
]=]
)

install(
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt"
        "${CMAKE_CURRENT_SOURCE_DIR}/idf_component.yml"
        "${CMAKE_CURRENT_SOURCE_DIR}/asn_module_config.hpp"
    DESTINATION "main"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/build_info.h"
    DESTINATION "main"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
)

install(
    DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}/components"
        "${CMAKE_CURRENT_SOURCE_DIR}/database"
        "${CMAKE_CURRENT_SOURCE_DIR}/program"
    DESTINATION "main"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.h"
        PATTERN "*.cpp"
        PATTERN "*.c"
)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-core/"
    DESTINATION "main/asn/asn-core"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.h"
)

install(
    FILES "${ASN_EXPORT_CMAKE_DIR}/asn-core-CMakeLists.txt"
    DESTINATION "main/asn/asn-core"
    RENAME "CMakeLists.txt"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-drivers/include/"
    DESTINATION "main/asn/asn-drivers/include"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.h"
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-drivers)
    install(
        FILES "$<TARGET_FILE:asn_drivers>"
        DESTINATION "main/asn/asn-drivers/lib"
        COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    )
endif()

install(
    FILES "${ASN_EXPORT_CMAKE_DIR}/asn-drivers-CMakeLists.txt"
    DESTINATION "main/asn/asn-drivers"
    RENAME "CMakeLists.txt"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-expander-lib/include/"
    DESTINATION "main/asn/asn-expander-lib/include"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.h"
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-expander-lib)
    install(
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-expander-lib/asn-expander-firmware.bin"
        DESTINATION "main/asn/asn-expander-lib"
        COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    )

    install(
        FILES "$<TARGET_FILE:asn_expander_lib>"
        DESTINATION "main/asn/asn-expander-lib/lib"
        COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    )

    install(
        FILES "$<TARGET_FILE:asn_expander_data>"
        DESTINATION "main/asn/asn-expander-lib/lib"
        COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    )
endif()

install(
    FILES "${ASN_EXPORT_CMAKE_DIR}/asn-expander-lib-CMakeLists.txt"
    DESTINATION "main/asn/asn-expander-lib"
    RENAME "CMakeLists.txt"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-esp32-modbus/include/"
    DESTINATION "main/asn/asn-esp32-modbus/include"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.h"
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-esp32-modbus)
    install(
        FILES "$<TARGET_FILE:asn_esp32_modbus>"
        DESTINATION "main/asn/asn-esp32-modbus/lib"
        COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    )
endif()

install(
    FILES "${ASN_EXPORT_CMAKE_DIR}/asn-esp32-modbus-CMakeLists.txt"
    DESTINATION "main/asn/asn-esp32-modbus"
    RENAME "CMakeLists.txt"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-eg915-driver/include/"
    DESTINATION "main/asn/asn-eg915-driver/include"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.h"
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-eg915-driver)
    install(
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-eg915-driver/at_parser_config.hpp"
        DESTINATION "main/asn/asn-eg915-driver"
        COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    )

    install(
        FILES "$<TARGET_FILE:asn_eg915_driver>"
        DESTINATION "main/asn/asn-eg915-driver/lib"
        COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    )
endif()

install(
    FILES "${ASN_EXPORT_CMAKE_DIR}/asn-eg915-driver-CMakeLists.txt"
    DESTINATION "main/asn/asn-eg915-driver"
    RENAME "CMakeLists.txt"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-eg915-driver/asn-at-parser/include/"
    DESTINATION "main/asn/asn-eg915-driver/asn-at-parser/include"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.h"
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-eg915-driver)
    install(
        FILES "$<TARGET_FILE:asn_at_parser>"
        DESTINATION "main/asn/asn-eg915-driver/asn-at-parser/lib"
        COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    )
endif()

install(
    FILES "${ASN_EXPORT_CMAKE_DIR}/asn-eg915-driver-asn-at-parser-CMakeLists.txt"
    DESTINATION "main/asn/asn-eg915-driver/asn-at-parser"
    RENAME "CMakeLists.txt"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-hal/include/"
    DESTINATION "main/asn/asn-hal/include"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-hal)
    install(
        FILES "$<TARGET_FILE:asn_hal>"
        DESTINATION "main/asn/asn-hal/lib"
        COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    )
endif()

install(
    FILES "${ASN_EXPORT_CMAKE_DIR}/asn-hal-CMakeLists.txt"
    DESTINATION "main/asn/asn-hal"
    RENAME "CMakeLists.txt"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-esp32-hal/include/"
    DESTINATION "main/asn/asn-esp32-hal/include"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-esp32-hal/old/"
    DESTINATION "main/asn/asn-esp32-hal/old"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.h"
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-esp32-hal)
    install(
        FILES "$<TARGET_FILE:asn_esp32_hal>"
        DESTINATION "main/asn/asn-esp32-hal/lib"
        COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    )
endif()

install(
    FILES "${ASN_EXPORT_CMAKE_DIR}/asn-esp32-hal-CMakeLists.txt"
    DESTINATION "main/asn/asn-esp32-hal"
    RENAME "CMakeLists.txt"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-esp32-ble/include/"
    DESTINATION "main/asn/asn-esp32-ble/include"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-esp32-ble)
    install(
        FILES "$<TARGET_FILE:asn_esp32_ble>"
        DESTINATION "main/asn/asn-esp32-ble/lib"
        COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    )
endif()

install(
    FILES "${ASN_EXPORT_CMAKE_DIR}/asn-esp32-ble-CMakeLists.txt"
    DESTINATION "main/asn/asn-esp32-ble"
    RENAME "CMakeLists.txt"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-esp32-wifi/include/"
    DESTINATION "main/asn/asn-esp32-wifi/include"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-esp32-wifi/old/"
    DESTINATION "main/asn/asn-esp32-wifi/old"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.h"
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/asn/asn-esp32-wifi)
    install(
        FILES "$<TARGET_FILE:asn_esp32_wifi>"
        DESTINATION "main/asn/asn-esp32-wifi/lib"
        COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    )
endif()

install(
    FILES "${ASN_EXPORT_CMAKE_DIR}/asn-esp32-wifi-CMakeLists.txt"
    DESTINATION "main/asn/asn-esp32-wifi"
    RENAME "CMakeLists.txt"
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
)
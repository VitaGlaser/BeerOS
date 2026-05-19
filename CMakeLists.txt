cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(revosoft-beerhouse-firmware)

set(ASN_CUSTOMER_EXPORT_COMPONENT customer-export)

install(
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt"
        "${CMAKE_CURRENT_SOURCE_DIR}/dependencies.lock"
        "${CMAKE_CURRENT_SOURCE_DIR}/partitions.csv"
        "${CMAKE_CURRENT_SOURCE_DIR}/sdkconfig.defaults"
    DESTINATION "."
    COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.clang-format")
    install(
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/.clang-format"
        DESTINATION "."
        COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.clangd")
    install(
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/.clangd"
        DESTINATION "."
        COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.vscode")
    install(
        DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/.vscode"
        DESTINATION "."
        COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.devcontainer")
    install(
        DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/.devcontainer"
        DESTINATION "."
        COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    )
endif()

file(GLOB ASN_API_DOCS "${CMAKE_CURRENT_SOURCE_DIR}/docs/api/*.yaml")
if(ASN_API_DOCS)
    install(
        FILES ${ASN_API_DOCS}
        DESTINATION "docs/api"
        COMPONENT ${ASN_CUSTOMER_EXPORT_COMPONENT}
    )
endif()
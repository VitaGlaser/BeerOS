idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS ".")

execute_process(COMMAND ${CMAKE_COMMAND} -E echo "Generating build_info.h")
string(TIMESTAMP BUILD_TIME "%Y-%m-%d %H:%M:%S" UTC)
string(TIMESTAMP BUILD_UNIX_SECS "%s" UTC)
math(EXPR BUILD_TIME_MS "${BUILD_UNIX_SECS} * 1000")
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/build_info.h
"#pragma once\n"
"#define BUILD_TIMESTAMP \"${BUILD_TIME}\"\n"
"#define BUILD_TIMESTAMP_MS ${BUILD_TIME_MS}\n"
)


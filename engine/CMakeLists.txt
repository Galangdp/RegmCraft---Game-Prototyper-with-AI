cmake_minimum_required(VERSION 3.10)
project(Regmcraft VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -flto")

add_compile_options(-Wall -Werror)

add_library(global_interface INTERFACE)

target_compile_definitions(global_interface INTERFACE
    "$<$<CONFIG:Debug>:RC_DEBUG>"
    "CRC_OREF"
)

target_include_directories(global_interface INTERFACE "${CMAKE_SOURCE_DIR}/include/shared")

add_subdirectory("source/extern")
add_subdirectory("source/shared")
add_subdirectory("source/app")

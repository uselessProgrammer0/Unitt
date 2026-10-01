cmake_minimum_required(VERSION 3.20)
project(Unitt)

set(CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(INCLUDE_DIRECTORY include)
set(SOURCE_DIRECTORY src)

# add_library(Unitt STATIC
#                ${INCLUDE_DIRECTORY}/expect.hpp
#                ${INCLUDE_DIRECTORY}/unit_test.hpp
#                ${INCLUDE_DIRECTORY}/sdarr.hpp
#                ${INCLUDE_DIRECTORY}/pdarr.hpp
#                ${SOURCE_DIRECTORY}/unit_test.cpp)

add_executable(Unitt
               ${INCLUDE_DIRECTORY}/assert.hpp
               ${INCLUDE_DIRECTORY}/color_mapper.hpp
               ${INCLUDE_DIRECTORY}/encoder.hpp
               ${INCLUDE_DIRECTORY}/execution.hpp
               ${INCLUDE_DIRECTORY}/fixture.hpp
               ${INCLUDE_DIRECTORY}/ioparr.hpp
               ${INCLUDE_DIRECTORY}/partitioner.hpp
               ${INCLUDE_DIRECTORY}/platform/platform.hpp
               ${INCLUDE_DIRECTORY}/platform/console.hpp
               ${INCLUDE_DIRECTORY}/register.hpp
               ${INCLUDE_DIRECTORY}/spdarr.hpp
               ${INCLUDE_DIRECTORY}/test_macros.hpp
               ${INCLUDE_DIRECTORY}/unit_test.hpp
               ${SOURCE_DIRECTORY}/unit_test.cpp
               ${SOURCE_DIRECTORY}/platform/platform.cpp)

set_property(TARGET Unitt PROPERTY CXX_STANDARD 20)
set_property(TARGET Unitt PROPERTY CXX_STANDARD_REQUIRED ON)

set(MSVC_OPTIONS /Zc:preprocessor)
add_compile_options(${MSVC_OPTIONS})
target_compile_options(Unitt PRIVATE ${MSVC_OPTIONS})

set_target_properties(Unitt PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

target_include_directories(Unitt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_DIRECTORY})
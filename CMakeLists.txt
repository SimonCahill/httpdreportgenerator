cmake_minimum_required(VERSION 3.12)

project(httpd-hit-report LANGUAGES CXX VERSION 0.0.1 DESCRIPTION "A simple utility to parse Apache logs and generate a report.")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT DEFINED httpdreport_CONFIGURED_OS OR httpdreport_CONFIGURED_OS STREQUAL "")
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ubuntu-defaults.cmake)
else()
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/${httpdreport_CONFIGURED_OS}-defaults.cmake)
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# First setup configure_files
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/resources/Resources.in.hpp ${CMAKE_CURRENT_BINARY_DIR}/include/resources/Resources.hpp)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${CMAKE_CURRENT_BINARY_DIR}/include/
)

file(GLOB_RECURSE FILES FOLLOW_SYMLINKS ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)

add_executable(${PROJECT_NAME} ${FILES})

target_link_libraries(
    ${PROJECT_NAME}

    fmt # requires libfmt-dev!
)

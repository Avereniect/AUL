cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if (${CMAKE_VERSION} VERSION_LESS 3.14)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.14)
endif()

project(
    AUL
    VERSION 0.0.1
    DESCRIPTION "A utility library"
    LANGUAGES CXX
)

#======================================
# A Utility Library
#======================================

add_library(AUL INTERFACE)

target_include_directories(AUL INTERFACE ./include/)
target_compile_features(AUL INTERFACE cxx_std_17)

option(AUL_BUILD_TESTS OFF)

if (AUL_BUILD_TESTS)
    add_subdirectory(./tests/  EXCLUDE_FROM_ALL)
    add_subdirectory(./external/googletest/  EXCLUDE_FROM_ALL)
endif()

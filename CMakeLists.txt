cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if (${CMAKE_VERSION} VERSION_LESS 3.14)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.14)
endif()

SET(CMAKE_BUILD_TYPE Debug)

project(AUL
    VERSION 0.0.1
    DESCRIPTION "A utility library"
    LANGUAGES CXX
)

include_directories(aul)

#add_subdirectory(vendor/googletest)

add_subdirectory(vendor/Celero)

add_library(AUL INTERFACE)

add_executable(AUL_BENCHMARKS ./benchmarks/AUL_benchmarks.cpp)
target_link_libraries(AUL_BENCHMARKS PUBLIC AUL celero)

add_executable(AUL_TESTS ./tests/AUL_tests.cpp)
target_link_libraries(AUL_TESTS PUBLIC AUL gtest gtest_main pthread)

target_compile_features(AUL_TESTS PUBLIC cxx_std_17)

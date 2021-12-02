cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if (${CMAKE_VERSION} VERSION_LESS 3.14)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.14)
endif()

#======================================
# AUL tests
#======================================

add_executable(AUL_TESTS ./AUL_tests.cpp)

target_link_libraries(AUL_TESTS PUBLIC AUL gtest gtest_main pthread)
target_compile_features(AUL_TESTS PRIVATE cxx_std_17)

cmake_minimum_required(VERSION 3.14)
project(AUL)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)
include_directories(src/containers)
include_directories(src/memory)

add_executable(AUL
        src/containers/Slot_map.hpp
        src/containers/Vector.hpp
        src/memory/Memory.hpp
        src/AUL.cpp
        src/Debug_class.cpp
        src/Debug_class.hpp
        src/Hacks.cpp
        src/Math.hpp
        src/Versioned_type.hpp)

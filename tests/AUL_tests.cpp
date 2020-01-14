//#include "Algorithms_tests.hpp"
#include "containers/Slot_map_tests.hpp"
//#include "memory/Linear_allocator_tests.hpp"

#include <gtest/gtest.h>

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

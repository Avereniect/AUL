//#include "containers/Array_map_tests.hpp"
//#include "containers/Circular_array_tests.hpp"
//#include "containers/Matrix_tests.hpp"
//#include "containers/Random_access_iterator_tests.hpp"
#include "containers/Slot_map_tests.hpp"

//#include "memory/Memory_tests.hpp"

//#include "Algorithms_tests.hpp"
//#include "Bit_tests.hpp"
//#include "Math_tests.hpp"
//#include "Utility_tests.hpp"

#include <gtest/gtest.h>

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

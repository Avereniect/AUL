#ifndef AUL_TESTS_ALGORITHMS_TESTS_HPP
#define AUL_TESTS_ALGORITHMS_TESTS_HPP

#include "../aul/Algorithms.hpp"

#include <gtest/gtest.h>
#include <algorithm>

namespace aul::tests {

    TEST(aul_linear_search, empty) {
        std::vector<float> vec;

        EXPECT_EQ(aul::linear_search(vec.begin(), vec.end(), 4.0), vec.end());
    }

    TEST(aul_linear_search, one_element) {
        std::vector<int> vec;
        vec.push_back(80);

        EXPECT_EQ(aul::linear_search(vec.begin(), vec.end(), 120), vec.end());
        EXPECT_EQ(aul::linear_search(vec.begin(), vec.end(), 80), vec.begin());
        EXPECT_EQ(aul::linear_search(vec.begin(), vec.end(), 40), vec.begin());
    }

    TEST(aul_linear_search, multiple_elements) {
        std::vector<int> vec = {8, 16, 32, 64};

        EXPECT_EQ(aul::linear_search(vec.begin(), vec.end(), 0), vec.begin());
        EXPECT_EQ(aul::linear_search(vec.begin(), vec.end(), 128), vec.end());

        EXPECT_EQ(aul::linear_search(vec.begin(), vec.end(), 64), vec.begin() + 3);
        EXPECT_EQ(aul::linear_search(vec.begin(), vec.end(), 32), vec.begin() + 2);
        EXPECT_EQ(aul::linear_search(vec.begin(), vec.end(), 16), vec.begin() + 1);
        EXPECT_EQ(aul::linear_search(vec.begin(), vec.end(),  8), vec.begin() + 0);
    }

}

#endif //AUL_TESTS_ALGORITHMS_TESTS_HPP

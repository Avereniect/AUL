#ifndef AUL_TESTS_ALGORITHMS_TESTS_HPP
#define AUL_TESTS_ALGORITHMS_TESTS_HPP

#include <aul/Algorithms.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <numeric>

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

    TEST(aul_binary_search, empty) {
        std::vector<int> vec;
        EXPECT_EQ(vec.begin(), aul::binary_search(vec.begin(), vec.end(), 0));
    }

    TEST(aul_binary_search, single_element) {
        std::vector<int> vec{1};

        EXPECT_EQ(vec.begin(), aul::binary_search(vec.begin(), vec.end(), 0));
        EXPECT_EQ(vec.begin(), aul::binary_search(vec.begin(), vec.end(), 1));
        EXPECT_EQ(vec.end(), aul::binary_search(vec.begin(), vec.end(), 2));
    }

    TEST(aul_binary_search, multiple_elements) {
        for (int i = 1; i < 64; ++i) {
             std::vector<int> vec;
            vec.resize(i);
            std::iota(vec.begin(), vec.end(), 0);

            EXPECT_EQ(vec.begin(), aul::binary_search(vec.begin(), vec.end(), vec.front() - 1));

            for (int j = 0; j < vec.size(); ++j) {
                auto iter = aul::binary_search(vec.begin(), vec.end(), j);
                bool result = (*iter == vec[j   ]);
                EXPECT_TRUE(result);
            }

            EXPECT_EQ(vec.end(), aul::binary_search(vec.begin(), vec.end(), vec.back() + 1));
        }
    }

    TEST(aul_binary_search, multiple_elementes_2N) {
        std::vector<int> vec{
            0x0, 0x1, 0x2, 0x3,
            0x4, 0x5, 0x6, 0x7
        };

        EXPECT_EQ(vec.begin(), aul::binary_search(vec.begin(), vec.end(), -1));
        for (int i = 0; i < 8; ++i) {
            EXPECT_EQ(vec.begin() + i, aul::binary_search(vec.begin(), vec.end(), i));
        }
        EXPECT_EQ(vec.end(), aul::binary_search(vec.begin(), vec.end(), 8));
    }

    TEST(aul_binary_search, single_element_less_than) {
        const std::vector<int> vec = {
            4, 3, 2, 1
        };

        auto comparator = std::greater<int>{};

        EXPECT_EQ(vec.begin(), aul::binary_search(vec.begin(), vec.end(), 5, comparator));
        EXPECT_EQ(vec.begin() + 0, aul::binary_search(vec.begin(), vec.end(), 4, comparator));
        EXPECT_EQ(vec.begin() + 1, aul::binary_search(vec.begin(), vec.end(), 3, comparator));
        EXPECT_EQ(vec.begin() + 2, aul::binary_search(vec.begin(), vec.end(), 2, comparator));
        EXPECT_EQ(vec.begin() + 3, aul::binary_search(vec.begin(), vec.end(), 1, comparator));
        EXPECT_EQ(vec.end(), aul::binary_search(vec.begin(), vec.end(), -1, comparator));
    }

    TEST(aul_binary_search, Multiple_equal_elements) {
        const std::vector<int> vec = {
            4, 4, 4, 4
        };

        EXPECT_EQ(vec.begin(), aul::binary_search(vec.begin(), vec.end(), 4));
    }

}

#endif //AUL_TESTS_ALGORITHMS_TESTS_HPP

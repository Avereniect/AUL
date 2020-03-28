#ifndef AUL_ASSOCIATIVE_LIST_TESTS_HPP
#define AUL_ASSOCIATIVE_LIST_TESTS_HPP


#include "../../aul/containers/Array_map.hpp"
#include <gtest/gtest.h>
#include <string>

namespace aul::tests {

    TEST(Array_map, Default_constructor) {
        aul::Array_map<int, float> arr;

        EXPECT_TRUE(arr.empty());
        EXPECT_EQ(arr.size(), 0);

        EXPECT_EQ(arr.begin(), arr.end());
        EXPECT_EQ(arr.data(), nullptr);
        EXPECT_EQ(arr.keys(), nullptr);

        EXPECT_ANY_THROW(arr.at(0));
    }

    TEST(Array_map, Insert) {
        aul::Array_map<int, int> arr{};
        arr.reserve(16);

        arr.insert(5, 16);
        arr.insert(6, 24);
        arr.insert(7, 32);
        arr.insert(8, 48);

        EXPECT_EQ(arr[5], 16);
        EXPECT_EQ(arr[6], 24);
        EXPECT_EQ(arr[7], 32);
        EXPECT_EQ(arr[8], 48);
    }

    TEST(Array_map, Erase_all) {
        aul::Array_map<int, int> arr{};

        arr.insert(16, 160);
        arr.insert(17, 170);
        arr.insert(18, 180);
        arr.insert(19, 190);
        arr.insert(20, 200);

        arr.erase(arr.begin() + 4);
        arr.erase(arr.begin() + 3);
        arr.erase(arr.begin() + 2);
        arr.erase(arr.begin() + 1);
        arr.erase(arr.begin() + 0);

        EXPECT_TRUE(arr.empty());
        EXPECT_EQ(arr.begin(), arr.end());
        EXPECT_EQ(arr.size(), 0);
        EXPECT_ANY_THROW(arr.at(16));
        EXPECT_ANY_THROW(arr.at(17));
        EXPECT_ANY_THROW(arr.at(18));
        EXPECT_ANY_THROW(arr.at(19));
        EXPECT_ANY_THROW(arr.at(20));
    }

}

#endif //AUL_ASSOCIATIVE_LIST_HPP

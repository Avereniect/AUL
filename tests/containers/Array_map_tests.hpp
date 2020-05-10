#ifndef AUL_ASSOCIATIVE_LIST_TESTS_HPP
#define AUL_ASSOCIATIVE_LIST_TESTS_HPP


#include "../../aul/containers/Array_map.hpp"
#include <gtest/gtest.h>
#include <string>

namespace aul::tests {

    //=====================================================
    // Text fixtures
    //=====================================================

    class Single_array_map : public ::testing::Test {
    protected:

        virtual void SetUp() override {
            map.insert(0, 0.00);
            map.insert(1, 1.00);
            map.insert(2, 2.00);
            map.insert(3, 3.00);
            map.insert(4, 4.00);
            map.insert(5, 5.00);
            map.insert(6, 6.00);
            map.insert(7, 7.00);
        }

        aul::Array_map<int, float> map;

    };

    class Two_equal_array_maps : public ::testing::Test {
    protected:

        virtual void SetUp() override {
            map0.insert(0, 0.00);
            map0.insert(1, 1.00);
            map0.insert(2, 2.00);
            map0.insert(3, 3.00);
            map0.insert(4, 4.00);
            map0.insert(5, 5.00);
            map0.insert(6, 6.00);
            map0.insert(7, 7.00);

            map1.insert(0, 0.00);
            map1.insert(1, 1.00);
            map1.insert(2, 2.00);
            map1.insert(3, 3.00);
            map1.insert(4, 4.00);
            map1.insert(5, 5.00);
            map1.insert(6, 6.00);
            map1.insert(7, 7.00);
        }

        aul::Array_map<int, float> map0;
        aul::Array_map<int, float> map1;
    };

    class Two_array_maps : public ::testing::Test {
    protected:

        virtual void SetUp() override {
            map0.insert(0, 0.00);
            map0.insert(1, 1.00);
            map0.insert(2, 2.00);
            map0.insert(3, 3.00);
            map0.insert(4, 4.00);
            map0.insert(5, 5.00);
            map0.insert(6, 6.00);
            map0.insert(7, 7.00);

            map0.insert(-0, -0.00);
            map0.insert(-1, -1.00);
            map0.insert(-2, -2.00);
            map0.insert(-3, -3.00);
            map0.insert(-4, -4.00);
            map0.insert(-5, -5.00);
            map0.insert(-6, -6.00);
            map0.insert(-7, -7.00);
        }

        aul::Array_map<int, float> map0;
        aul::Array_map<int, float> map1;
    };

    //=====================================================
    // -ctor tests
    //=====================================================

    TEST(Array_map, Default_constructor) {
        aul::Array_map<int, float> arr;

        EXPECT_TRUE(arr.empty());
        EXPECT_EQ(arr.size(), 0);

        EXPECT_EQ(arr.begin(), arr.end());
        EXPECT_EQ(arr.data(), nullptr);
        EXPECT_EQ(arr.keys(), nullptr);

        EXPECT_ANY_THROW(arr.at(0));
    }

    TEST(Associative_array, Move_constructor) {
        aul::Array_map<int, float> arr0;
        arr0.insert(4, 4.0);
        arr0.insert(5, 5.0);
        arr0.insert(6, 6.0);
        arr0.insert(7, 7.0);

        auto begin = arr0.begin();
        auto end = arr0.end();

        aul::Array_map<int, float> arr1{std::move(arr0)};

        EXPECT_TRUE(arr0.empty());
        EXPECT_EQ(arr0.size(), 0);
        EXPECT_EQ(arr0.begin(), arr0.end());

        EXPECT_FALSE(arr1.empty());
        EXPECT_EQ(arr1.size(), 4);
        EXPECT_EQ(arr1.begin(), begin);
        EXPECT_EQ(arr1.end(), end);
        EXPECT_GE(arr1.capacity(), 4);
    }

    TEST_F(Single_array_map, Copy_constructor) {
        auto map_copy{map};

        EXPECT_EQ(map_copy.size(), map.size());
        EXPECT_GE(map_copy.capacity(), map_copy.size());

        for (int i = 0; i < map.size(); ++i) {
            EXPECT_EQ(map[i], map_copy[i]);
        }

        EXPECT_EQ(map_copy.get_allocator(), map.get_allocator());
    }

    //=====================================================
    // Assignment operators
    //=====================================================

    TEST(Array_map, Move_assignment) {
        aul::Array_map<int, float> arr0;
        arr0.insert(0, 0.0);
        arr0.insert(1, 1.0);
        arr0.insert(2, 2.0);
        arr0.insert(3, 3.0);

        aul::Array_map<int, float> arr1;
        arr1.insert(4, 4.0);
        arr1.insert(5, 5.0);
        arr1.insert(6, 6.0);
        arr1.insert(7, 7.0);
    }

    TEST_F(Two_array_maps, Copy_assignment) {
        map1 = map0;

        EXPECT_EQ(map1.size(), map0.size());
        EXPECT_GE(map1.capacity(), map1.size());
        EXPECT_EQ(map1.get_allocator(), map0.get_allocator());

        for (int i = 0; i < map1.size(); ++i) {
            EXPECT_EQ(map1[i], map0[i]);
        }
    }

    //=====================================================
    // Element mutators
    //=====================================================

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

    //=====================================================
    // Comparison operators
    //=====================================================

    TEST_F(Two_equal_array_maps, Comparison_operators0) {
        EXPECT_TRUE(map0 == map1);
        EXPECT_FALSE(map0 != map1);
    }

    TEST_F(Two_array_maps, Comparison_operators1) {
        EXPECT_TRUE(map0 != map1);
        EXPECT_FALSE(map0 == map1);
    }

    //=====================================================
    // Misc.
    //=====================================================

    TEST_F(Single_array_map, clear) {
        map.clear();

        EXPECT_TRUE(map.empty());
        EXPECT_EQ(map.size(), 0);

        EXPECT_TRUE(map.begin() == map.end());
    }

}

#endif //AUL_ASSOCIATIVE_LIST_HPP

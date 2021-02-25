#ifndef AUL_ASSOCIATIVE_LIST_TESTS_HPP
#define AUL_ASSOCIATIVE_LIST_TESTS_HPP


#include <aul/containers/Array_map.hpp>

#include <gtest/gtest.h>
#include <string>
#include <iostream>

namespace aul::tests {

    //=====================================================
    // Test fixtures
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

        auto begin = arr0.begin();
        auto end = arr0.end();

        aul::Array_map<int, float> arr1;
        arr1 = std::move(arr0);

        EXPECT_EQ(arr1[0], 0.0);
        EXPECT_EQ(arr1[1], 1.0);
        EXPECT_EQ(arr1[2], 2.0);
        EXPECT_EQ(arr1[3], 3.0);

        EXPECT_EQ(arr1.size(), 4);
        EXPECT_EQ(arr1.get_allocator(), arr0.get_allocator());

        EXPECT_EQ(arr1.begin(), begin);
        EXPECT_EQ(arr1.end(), end);
    }

    TEST(Array_map, Copy_assignment) {
        aul::Array_map<int16_t, int16_t> map0;
        aul::Array_map<int16_t, int16_t> map1;

        map0.insert(0x00, 0x00);
        map0.insert(0x01, 0x01);
        map0.insert(0x02, 0x02);
        map0.insert(0x03, 0x03);
        map0.insert(0x04, 0x04);
        map0.insert(0x05, 0x05);
        map0.insert(0x06, 0x06);
        map0.insert(0x07, 0x07);

        map0.insert(0xFFFF, 0xFFFF);
        map0.insert(0xFFFE, 0xFFFE);
        map0.insert(0xFFFD, 0xFFFD);
        map0.insert(0xFFFC, 0xFFFC);
        map0.insert(0xFFFB, 0xFFFB);
        map0.insert(0xFFFA, 0xFFFA);

        map1 = map0;

        EXPECT_EQ(map1.size(), map0.size());
        EXPECT_GE(map1.capacity(), map1.size());
        EXPECT_EQ(map1.get_allocator(), map0.get_allocator());

        for (int i = 0; i < 8; ++i) {
            EXPECT_EQ(map1[i], map0[i]);
        }
        for (int i = 0xFFFA; i < 0; ++i) {
            EXPECT_EQ(map1[i], map0[i]);
        }
    }

    //=====================================================
    // Element mutators
    //=====================================================

    TEST(Array_map, Emplace) {
        aul::Array_map<int, float> arr{};
        arr.emplace(0, 0.0);
        arr.emplace(1, 1.0);
        arr.emplace(2, 2.0);

        EXPECT_EQ(arr[0], 0.0);
        EXPECT_EQ(arr[1], 1.0);
        EXPECT_EQ(arr[2], 2.0);

        arr.clear();
        arr.reserve(8);

        arr.emplace(0, 0.0);
        arr.emplace(1, 1.0);
        arr.emplace(2, 2.0);
        arr.emplace(3, 3.0);

        EXPECT_EQ(arr[0], 0.0);
        EXPECT_EQ(arr[1], 1.0);
        EXPECT_EQ(arr[2], 2.0);
        EXPECT_EQ(arr[3], 3.0);
    }

    TEST(Array_map, Insert) {
        aul::Array_map<int, int> arr{};
        arr.reserve(16);

        arr.insert(5, 16);
        arr.insert(6, 24);
        arr.insert(7, 32);
        arr.insert(8, 48);

        arr.insert(0, 0);
        arr.insert(-1, -1);
        arr.insert(-2, -2);
        arr.insert(-3, -3);

        EXPECT_EQ(arr[0], 0);
        EXPECT_EQ(arr[-1], -1);
        EXPECT_EQ(arr[-2], -2);
        EXPECT_EQ(arr[5], 16);
        EXPECT_EQ(arr[6], 24);
        EXPECT_EQ(arr[7], 32);
        EXPECT_EQ(arr[8], 48);
    }

    TEST(Array_map, Erase) {
        aul::Array_map<int, float> arr{};
        EXPECT_EQ(arr.end(), arr.erase(0));

        arr.emplace(1, 56.0);
        EXPECT_EQ(arr.end(), arr.erase(0));
        EXPECT_EQ(arr.begin(), arr.erase(1));

        arr.emplace(0, 24.0);
        arr.emplace(1, 48.0);
        arr.emplace(2, 96.0);

        auto temp0 = arr.end() - 1;
        auto temp1 = arr.erase(2);

        EXPECT_EQ(temp0, temp1);

        temp0 = arr.end() - 1;
        temp1 = arr.erase(1);
        EXPECT_EQ(temp0, temp1);

        temp0 = arr.end() - 1;
        temp1 = arr.erase(0);
        EXPECT_EQ(temp0, temp1);

        EXPECT_EQ(arr.size(), 0);
    }

    TEST(Array_map, Erase_with_iterators_all) {
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

    TEST(Array_map, For_each) {
        aul::Array_map<int, float> map;

        for (auto x : map) {
            // Just making sure this doesn't segfault
        }

        map.emplace(0, 0.0);
        map.emplace(1, 1.0);
        map.emplace(2, 2.0);
        map.emplace(3, 3.0);

        std::vector<int> results;
        for (auto x : map) {
            results.push_back(x);
        }

        for (std::size_t i = 0; i < map.size(); ++i) {
            results[i] = map.data()[i];
        }

        map.clear();
        for (auto x : map) {
            // Just making sure this doesn't segfault
        }
    }

    //=====================================================
    // Inspection functions
    //=====================================================

    TEST(Array_map, Contains) {
        aul::Array_map<int, float> map;

        map.insert(1, 1.0);
        map.insert(2, 1.0);
        map.insert(3, 1.0);

        EXPECT_TRUE(map.contains(1));
        EXPECT_TRUE(map.contains(2));
        EXPECT_TRUE(map.contains(3));

        EXPECT_FALSE(map.contains(0));
        EXPECT_FALSE(map.contains(4));
    }

    TEST(Array_map, Find) {
        aul::Array_map<int, float> map;
        map.reserve(3);

        map.emplace(1, 1.0);
        map.emplace(2, 2.0);
        map.emplace(3, 3.0);
        map.emplace(4, 4.0);

        EXPECT_EQ(*map.find(1), 1.0);
        EXPECT_EQ(*map.find(2), 2.0);
        EXPECT_EQ(*map.find(3), 3.0);
        EXPECT_EQ(*map.find(4), 4.0);
    }

}

#endif //AUL_ASSOCIATIVE_LIST_HPP

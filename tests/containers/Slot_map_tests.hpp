#ifndef AUL_SLOT_MAP_TESTS_HPP
#define AUL_SLOT_MAP_TESTS_HPP

#include "../../aul/memory/Linear_allocator.hpp"
#include "../../aul/containers/Slot_map.hpp"

#include <gtest/gtest.h>

namespace aul::tests {

    //=====================================================
    // Test fixtures
    //=====================================================

    class Slot_map_list_f : public ::testing::Test {
    protected:

        virtual void SetUp() override {
            map = {1.0, 2.0, 3.0, 4.0};
        }

        aul::Slot_map<double> map;

    };

    class Slot_map_list_f2 : public ::testing::Test {

        virtual void SetUp() override {
            map0 = {0, 1, 2, 3};
            map1 = {0, 1, 2, 3};
        }

        aul::Slot_map<double> map0;
        aul::Slot_map<double> map1;
    };

    //=====================================================
    // -ctors
    //=====================================================

    TEST(Slot_map, Default_constructor) {
        aul::Slot_map<double> map;

        EXPECT_EQ(map.size(), 0);
        EXPECT_EQ(map.capacity(), 0);
        EXPECT_TRUE(map.empty());
        EXPECT_EQ(map.begin(), map.end());
        EXPECT_EQ(map.end() - map.begin(), 0);
        EXPECT_EQ(map.rbegin(), map.rend());
        EXPECT_EQ(map.rend() - map.rbegin(), 0);
    }

    TEST(Slot_map, Allocator_extended_default_contructor) {
        aul::Linear_allocator<double> allocator(1024);
        aul::Slot_map<double, decltype(allocator)> map(allocator);

        EXPECT_EQ(map.size(), 0);
        EXPECT_EQ(map.capacity(), 0);
        EXPECT_TRUE(map.empty());
        EXPECT_EQ(map.begin(), map.end());
        EXPECT_EQ(map.end() - map.begin(), 0);
        EXPECT_EQ(map.get_allocator(), allocator);
    }

    TEST(Slot_map, Fill_constructor) {
        using size_type = aul::Slot_map<double>::size_type;

        constexpr size_type size = 128;
        constexpr double value = 4.0;

        aul::Slot_map<double> map(size, value);

        auto equality_predicate = [value] (double x) -> bool {
            return value == x;
        };

        EXPECT_FALSE(map.empty());
        EXPECT_EQ(map.size(), size);
        EXPECT_GE(map.capacity(), size);
        EXPECT_TRUE(std::all_of(map.begin(), map.end(), equality_predicate));
    }

    TEST(Slot_map, Default_fill_constructor) {
        using size_type = aul::Slot_map<double>::size_type;

        constexpr size_type SIZE = 56;
        aul::Slot_map<double> map(SIZE);

        auto predicate = [](double x) -> bool {
            return x == double{};
        };

        EXPECT_FALSE(map.empty());
        EXPECT_EQ(map.size(), SIZE);
        EXPECT_GE(map.capacity(), SIZE);
        EXPECT_EQ(map.end() - map.begin(), 56);
        EXPECT_TRUE(std::all_of(map.begin(), map.end(), predicate));
    }

    TEST(Slot_map, Initializer_list_constructor) {
        const std::initializer_list<double> list = {
            1.0, 2.0, 4.0, 8.0,
            16.0, 32.0, 64.0, 128.0
        };

        using size_type = aul::Slot_map<double>::size_type;

        const size_type SIZE = list.size();
        aul::Slot_map<double> map{list};

        EXPECT_FALSE(map.empty());
        EXPECT_EQ(map.size(), SIZE);
        EXPECT_GE(map.capacity(), SIZE);
        EXPECT_TRUE(std::equal(list.begin(), list.end(), map.begin()));
    }

    TEST(Slot_map, Move_constructor) {
        std::initializer_list<double> list = {
            1.0, 2.0, 3.0, 4.0
        };

        aul::Slot_map<double> map0{list};
        aul::Slot_map<double> map1{std::move(map0)};

        EXPECT_TRUE(map0.empty());
        EXPECT_FALSE(map1.empty());

        EXPECT_EQ(map0.size(), 0);
        EXPECT_EQ(map1.size(), 4);

        EXPECT_EQ(map0.capacity(), 0);
        EXPECT_GE(map1.capacity(), 4);

        EXPECT_TRUE(std::equal(list.begin(), list.end(), map1.begin()));
    }

    //=====================================================
    // Comparison operator tests
    //=====================================================

    TEST(Slot_map, Comparisons) {
        aul::Slot_map<int> map0 = {0, 0, 0, 0};
        aul::Slot_map<int> map1 = {0, 0, 0, 0};
        aul::Slot_map<int> map2 = {0, 0, 0, 1};

        EXPECT_EQ(map0, map1);
        EXPECT_LE(map0, map1);
        EXPECT_GE(map0, map1);
        EXPECT_NE(map1, map2);
        EXPECT_GT(map2, map0);
        EXPECT_LT(map0, map2);
        EXPECT_GE(map2, map1);
        EXPECT_LE(map1, map2);
    }

    //=====================================================
    // Mutator tests
    //=====================================================

    TEST_F(Slot_map_list_f, clear) {
        map.clear();
        EXPECT_EQ(map.size(), 0);
        EXPECT_EQ(map.capacity(), 0);
        EXPECT_EQ(map.begin(), map.end());
    }

    TEST(Slot_map, reserve) {
        aul::Slot_map<double> map;

        map.reserve(1);
        map.reserve(2);
        map.reserve(4);
        map.reserve(8);
        map.reserve(16);
        map.reserve(24);
        map.reserve(8);
        map.reserve(48);
        map.reserve(64);
        map.reserve(256);
        map.reserve(0);

        EXPECT_TRUE(map.empty());
        EXPECT_EQ(map.size(), 0u);
        EXPECT_GE(map.capacity(), 512u);
    }

    /*
    TEST(Slot_map, push_back) {
        aul::Slot_map<int> map;

        auto key0 = map.push_back(17);
        auto key1 = map.push_back(908);
        auto key2 = map.push_back(0);
        auto key3 = map.push_back(-17);

        auto key4 = map.push_back(17);
        auto key5 = map.push_back(908);
        auto key6 = map.push_back(0);
        auto key7 = map.push_back(-17);

        EXPECT_EQ(map[key0], 17);
        EXPECT_EQ(map[0], 17);

        EXPECT_EQ(map[key1], 908);
        EXPECT_EQ(map[1], 908);

        EXPECT_EQ(map[key2], 0);
        EXPECT_EQ(map[2], 0);

        EXPECT_EQ(map[key3], -17);
        EXPECT_EQ(map[3], -17);

        EXPECT_EQ(map[key4], 17);
        EXPECT_EQ(map[4], 17);

        EXPECT_EQ(map[key5], 908);
        EXPECT_EQ(map[5], 908);

        EXPECT_EQ(map[key6], 0);
        EXPECT_EQ(map[6], 0);

        EXPECT_EQ(map[key7], -17);
        EXPECT_EQ(map[7], -17);
    }

    TEST(Slot_map, Emplace_back) {
        aul::Slot_map<int> map;
        auto key0 = map.emplace_back(17);
        auto key1 = map.emplace_back(908);
        auto key2 = map.emplace_back(0);
        auto key3 = map.emplace_back(-17);

        auto key4 = map.emplace_back(17);
        auto key5 = map.emplace_back(908);
        auto key6 = map.emplace_back(0);
        auto key7 = map.emplace_back(-17);

        EXPECT_EQ(map[key0], 17);
        EXPECT_EQ(map[0], 17);

        EXPECT_EQ(map[key1], 908);
        EXPECT_EQ(map[1], 908);

        EXPECT_EQ(map[key2], 0);
        EXPECT_EQ(map[2], 0);

        EXPECT_EQ(map[key3], -17);
        EXPECT_EQ(map[3], -17);

        EXPECT_EQ(map[key4], 17);
        EXPECT_EQ(map[4], 17);

        EXPECT_EQ(map[key5], 908);
        EXPECT_EQ(map[5], 908);

        EXPECT_EQ(map[key6], 0);
        EXPECT_EQ(map[6], 0);

        EXPECT_EQ(map[key7], -17);
        EXPECT_EQ(map[7], -17);
    }

    TEST_F(Slot_map_list_f, Pop_back) {
        map.pop_back();
        map.pop_back();
        map.pop_back();
        map.pop_back();

        EXPECT_TRUE(map.empty());
        EXPECT_EQ(map.size(), 0);
        EXPECT_GE(map.capacity(), 4);
        EXPECT_ANY_THROW(map.at(0));
        EXPECT_EQ(map.begin(), map.end());
    }

    TEST(Slot_map, Erase_key) {
        aul::Slot_map<long> map;

        auto key0 = map.push_back(0);
        auto key1 = map.push_back(1);
        auto key2 = map.push_back(2);
        auto key3 = map.push_back(3);

        map.erase(key0);
        map.erase(key1);
        map.erase(key2);
        map.erase(key3);

        map.push_back(5);
        map.push_back(6);
        map.push_back(7);
        map.push_back(8);


        map.erase(key3);
        map.erase(key2);
        map.erase(key1);
        map.erase(key0);
    }*/



}

#endif //AUL_SLOT_MAP_TESTS_HPP

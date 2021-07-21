#ifndef AUL_SLOT_MAP_TESTS_HPP
#define AUL_SLOT_MAP_TESTS_HPP

#include <aul/containers/Slot_map.hpp>

#include <gtest/gtest.h>

namespace aul::tests {

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
    }

    TEST(Slot_map, Move_constructor) {
        std::initializer_list<double> list = {
            1.0, 2.0, 3.0, 4.0
        };

        aul::Slot_map<double> map0;
        map0.emplace(1.0);
        map0.emplace(2.0);
        map0.emplace(3.0);
        map0.emplace(4.0);

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
    // Comparison operators
    //=====================================================

    TEST(Slot_map, Comparisons) {
        aul::Slot_map<uint32_t> map0;
        aul::Slot_map<uint32_t> map1;

        EXPECT_EQ(map0, map1);

        map0.emplace(86);
        EXPECT_NE(map0, map1);

        map1.emplace(86);
        EXPECT_EQ(map0, map1);
    }

    //=====================================================
    // Mutator tests
    //=====================================================

    TEST(Slot_map, Clear_empty) {
        aul::Slot_map<int> map;
        EXPECT_EQ(map.size(), 0);
        EXPECT_EQ(map.capacity(), 0);
        EXPECT_EQ(map.begin(), map.end());
    }

    TEST(Slot_map, Clear_with_elements) {
        aul::Slot_map<int> map;
        map.emplace(0);
        map.emplace(1);
        map.emplace(2);
        map.emplace(3);
        map.emplace(4);
        map.emplace(5);
        map.emplace(6);
        map.emplace(7);

        map.clear();

        EXPECT_TRUE(map.empty());
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
        EXPECT_GE(map.capacity(), 256);
    }

    TEST(Slot_map, Emplace) {
        aul::Slot_map<int32_t> map;

        std::vector<aul::Slot_map<int32_t>::key_type> keys;

        for (int32_t i = 0; i < 64; ++i) {
            keys.emplace_back(map.emplace(i));
        }

        EXPECT_FALSE(map.empty());

        for (int32_t i = 0; i < 64; ++i) {
            EXPECT_EQ(map.begin()[i], i);
            EXPECT_EQ(map[keys[i]], i);
        }
    }

    TEST(Slot_map, Erase_iterator) {
        aul::Slot_map<int> map;
        std::vector<decltype(map)::key_type> keys;

        keys.push_back(map.emplace(0));
        keys.push_back(map.emplace(1));
        keys.push_back(map.emplace(2));
        keys.push_back(map.emplace(3));

        map.erase(map.begin());

        EXPECT_EQ(map.size(), 3);
        EXPECT_EQ(map.begin()[0], 3);
        EXPECT_EQ(map.begin()[1], 1);
        EXPECT_EQ(map.begin()[2], 2);

        EXPECT_ANY_THROW(map.at(keys[0]));
        EXPECT_EQ(map[keys[1]], 1);
        EXPECT_EQ(map[keys[2]], 2);
        EXPECT_EQ(map[keys[3]], 3);
    }

    TEST(Slot_map, Erase_key) {
        aul::Slot_map<long> map;

        auto key0 = map.emplace(0);
        auto key1 = map.emplace(1);
        auto key2 = map.emplace(2);
        auto key3 = map.emplace(3);

        map.erase(key3);
        map.erase(key2);
        map.erase(key1);
        map.erase(key0);

        EXPECT_TRUE(map.empty());
    }

}

#endif //AUL_SLOT_MAP_TESTS_HPP

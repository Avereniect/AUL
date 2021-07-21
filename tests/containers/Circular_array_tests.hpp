//
// Created by avereniect on 5/10/20.
//

#ifndef AUL_CIRCULAR_ARRAY_TESTS_HPP
#define AUL_CIRCULAR_ARRAY_TESTS_HPP

#include <aul/containers/Circular_array.hpp>

#include <iostream>
#include <gtest/gtest.h>

namespace aul::tests {

    //=====================================================
    // -ctors
    //=====================================================

    TEST(Circular_array, Default_constructor) {
        Circular_array<float> arr{};

        EXPECT_EQ(arr.get_allocator(), std::allocator<float>{});
        EXPECT_EQ(arr.size(), 0);
        EXPECT_EQ(arr.capacity(), 0);
        EXPECT_TRUE(arr.empty());
        EXPECT_EQ(arr.begin(), arr.end());
        EXPECT_EQ(arr.cbegin(), arr.cend());
        EXPECT_EQ(arr.rbegin(), arr.rend());
        EXPECT_EQ(arr.crbegin(), arr.crend());

        EXPECT_ANY_THROW(arr.at(0));
        EXPECT_ANY_THROW(arr.at(1));
    }

    TEST(Circular_array, Initializer_list) {
        std::initializer_list<int> list = {0, 1, 2, 4, 8, 16, 32, 64};
        aul::Circular_array<int> arr{list};

        EXPECT_EQ(arr.get_allocator(), std::allocator<int>{});
        EXPECT_EQ(arr.size(), 8);
        EXPECT_GE(arr.capacity(), 8);
        EXPECT_FALSE(arr.empty());
        EXPECT_EQ(arr.end() - arr.begin(), 8);
        EXPECT_EQ(arr.rend() - arr.rbegin(), 8);

        for (std::size_t i  = 0; i != list.size(); ++i) {
            EXPECT_EQ(list.begin()[i], arr[i]);
        }


        for (std::size_t i = 0; i != list.size(); ++i) {
            EXPECT_EQ(list.begin()[i], arr.begin()[i]);
        }
    }

    //=================================================
    // Assignment operators/methods
    //=================================================

    TEST(Circular_array, Copy_assignment_empty) {
        aul::Circular_array<int> arr0{};
        aul::Circular_array<int> arr1{};
        arr1 = arr0;

        EXPECT_EQ(arr1.get_allocator(), std::allocator<float>{});
        EXPECT_EQ(arr1.size(), 0);
        EXPECT_EQ(arr1.capacity(), 0);
        EXPECT_TRUE(arr1.empty());
        EXPECT_EQ(arr1.begin(), arr1.end());
        EXPECT_EQ(arr1.cbegin(), arr1.cend());
        EXPECT_EQ(arr1.rbegin(), arr1.rend());
        EXPECT_EQ(arr1.crbegin(), arr1.crend());

        EXPECT_ANY_THROW(arr1.at(0));
        EXPECT_ANY_THROW(arr1.at(1));

        arr1 = aul::Circular_array<int>{0, 1, 2, 3, 4, 5, 6, 7};

        EXPECT_EQ(arr1.get_allocator(), std::allocator<float>{});
        EXPECT_EQ(arr1.size(), 8);
        EXPECT_GE(arr1.capacity(), 8);
        EXPECT_FALSE(arr1.empty());

        for (std::size_t i = 0; i != arr1.size(); ++i) {
            EXPECT_EQ(arr1[i], i);
        }
    }

    TEST(Circular_array, Move_assignment) {}

    TEST(Circular_array, Generic_assign_empty) {
        std::vector<int> vec{0, 1, 2, 4, 8, 16, 32, 64};
        
        aul::Circular_array<int> arr{};
        arr.assign(vec.begin(), vec.end());

        EXPECT_EQ(arr.size(), 8);
        EXPECT_GE(arr.capacity(), 8);
        EXPECT_EQ(arr.end() - arr.begin(), 8);
        EXPECT_EQ(arr.rend() - arr.rbegin(), 8);

        for (std::size_t i = 0; i != arr.size(); ++i) {
            EXPECT_EQ(vec[i], arr[i]);
        }
    }

    TEST(Circular_array, Generic_assign_with_contents) {
        std::vector<int> vec{ 0, 1, 2, 4, 8, 16, 32, 64 };

        aul::Circular_array<int> arr{0, 1, 2, 3, 4, 5, 6, 7};
        arr.assign(vec.begin(), vec.end());
    }

    TEST(Circular_array, Assign_empty) {}

    TEST(Circular_array, Assign_with_contents) {}

    //=====================================================
    // State mutators
    //=====================================================

    TEST(Circular_array, Reserve_default) {
        aul::Circular_array<int> arr;
        arr.reserve(1024);

        EXPECT_EQ(arr.get_allocator(), std::allocator<int>{});
        EXPECT_EQ(arr.size(), 0);
        EXPECT_GE(arr.capacity(), 1024);
        EXPECT_TRUE(arr.empty());
        EXPECT_EQ(arr.begin(), arr.end());

        EXPECT_ANY_THROW(arr.at(0));
        EXPECT_ANY_THROW(arr.at(1));
        EXPECT_ANY_THROW(arr.at(2));
    }

    TEST(Circular_array, Reserve_with_elements) {
        std::initializer_list<int> list{0, 1, 2, 4, 8, 16, 32, 64};
        aul::Circular_array<int> arr{list};
        arr.reserve(1024);

        EXPECT_EQ(arr.get_allocator(), std::allocator<int>{});
        EXPECT_EQ(arr.size(), list.size());
        EXPECT_GE(arr.capacity(), 1024);
        EXPECT_FALSE(arr.empty());
        EXPECT_EQ(arr.end() - arr.begin(), list.size());

        for (std::size_t i = 0; i < list.size(); ++i) {
            EXPECT_EQ(arr[i], list.begin()[i]);
        }
    }

    //=====================================================
    // Element addition
    //=====================================================

    TEST(Circular_array, Emplace_front) {
        Circular_array<int> arr{};

        arr.emplace_front(128);
        arr.emplace_front(64);
        arr.emplace_front(32);
        arr.emplace_front(16);
        arr.emplace_front(8);
        arr.emplace_front(4);
        arr.emplace_front(2);
        arr.emplace_front(1);

        EXPECT_EQ(arr.get_allocator(), std::allocator<int>{});
        EXPECT_EQ(arr.size(), 8);
        EXPECT_GE(arr.capacity(), 8);
        EXPECT_EQ(arr.end() - arr.begin(), 8);
        EXPECT_FALSE(arr.empty());

        for (std::size_t i = 0; i < 8; ++i) {
            EXPECT_EQ(arr[i], 1 << i);
        }

    }

    TEST(Circular_array, Emplace_back) {
        Circular_array<int> arr{};

        arr.emplace_back(1);
        arr.emplace_back(2);
        arr.emplace_back(4);
        arr.emplace_back(8);
        arr.emplace_back(16);
        arr.emplace_back(32);
        arr.emplace_back(64);
        arr.emplace_back(128);

        EXPECT_EQ(arr.get_allocator(), std::allocator<int>{});
        EXPECT_EQ(arr.size(), 8);
        EXPECT_GE(arr.capacity(), 8);
        EXPECT_EQ(arr.end() - arr.begin(), 8);
        EXPECT_FALSE(arr.empty());

        for (std::size_t i = 0; i < 8; ++i) {
            EXPECT_EQ(arr[i], 1 << i);
        }

    }

    /*
    TEST(Circular_array, Emplace) {
        Circular_array<int> arr{};

        arr.emplace(arr.begin(), 0x44);
        arr.emplace(arr.begin(), 0x45);
        arr.emplace(arr.begin(), 0x46);
        arr.emplace(arr.begin(), 0x47);

        EXPECT_EQ(arr[0], 0x47);
        EXPECT_EQ(arr[1], 0x46);
        EXPECT_EQ(arr[2], 0x45);
        EXPECT_EQ(arr[3], 0x44);
    }
    */

    //=====================================================
    // Element removal
    //=====================================================

    TEST(Circular_array, Pop_back) {
        std::initializer_list<int> list{ 0, 1, 2, 4, 8, 16, 32, 64 };
        aul::Circular_array<int> arr{list};

        arr.pop_back();
        arr.pop_back();
        arr.pop_back();
        arr.pop_back();

        arr.pop_back();
        arr.pop_back();
        arr.pop_back();
        arr.pop_back();

        EXPECT_EQ(arr.get_allocator(), std::allocator<float>{});
        EXPECT_EQ(arr.size(), 0);
        EXPECT_GE(arr.capacity(), 8);
        EXPECT_TRUE(arr.empty());
        EXPECT_EQ(arr.begin(), arr.end());

        EXPECT_ANY_THROW(arr.at(0));
        EXPECT_ANY_THROW(arr.at(1));
    }

    TEST(Circular_array, Pop_front) {
        std::initializer_list<int> list{ 0, 1, 2, 4, 8, 16, 32, 64 };
        aul::Circular_array<int> arr{ list };

        arr.pop_front();
        arr.pop_front();
        arr.pop_front();
        arr.pop_front();

        arr.pop_front();
        arr.pop_front();
        arr.pop_front();
        arr.pop_front();

        EXPECT_EQ(arr.get_allocator(), std::allocator<float>{});
        EXPECT_EQ(arr.size(), 0);
        EXPECT_GE(arr.capacity(), 8);
        EXPECT_TRUE(arr.empty());
        EXPECT_EQ(arr.begin(), arr.end());

        EXPECT_ANY_THROW(arr.at(0));
        EXPECT_ANY_THROW(arr.at(1));
    }

    //=====================================================
    // Misc. functions
    //=====================================================

    TEST(Circular_array, Clear_default_constructed) {
        Circular_array<float> arr{};
        arr.clear();

        EXPECT_EQ(arr.get_allocator(), std::allocator<float>{});
        EXPECT_EQ(arr.size(), 0);
        EXPECT_EQ(arr.capacity(), 0);
        EXPECT_TRUE(arr.empty());
        EXPECT_EQ(arr.begin(), arr.end());

        EXPECT_ANY_THROW(arr.at(0));
        EXPECT_ANY_THROW(arr.at(1));
        EXPECT_ANY_THROW(arr.at(2));
    }

    TEST(Circular_array, Clear_with_elements) {
        std::initializer_list<int> list{0, 1, 2, 4, 8, 16, 32, 64};
        aul::Circular_array<int> arr{list};
        arr.clear();

        EXPECT_EQ(arr.get_allocator(), std::allocator<float>{});
        EXPECT_EQ(arr.size(), 0);
        EXPECT_EQ(arr.capacity(), 0);
        EXPECT_TRUE(arr.empty());
        EXPECT_EQ(arr.begin(), arr.end());

        EXPECT_ANY_THROW(arr.at(0));
        EXPECT_ANY_THROW(arr.at(1));
        EXPECT_ANY_THROW(arr.at(2));
    }

    //=====================================================
    // Features
    //=====================================================

    /*
    TEST(Circular_array, Constexpr_support) {
        constexpr aul::Circular_array<int> arr0{};
        constexpr aul::Circular_array<int> arr1{};
    }
     */

}

#endif //AUL_CIRCULAR_ARRAY_TESTS_HPP

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
        EXPECT_EQ(arr.cend() - arr.cbegin(), 8);
        EXPECT_EQ(arr.rend() - arr.rbegin(), 8);

        for (std::size_t i  = 0; i != list.size(); ++i) {
            int u = list.begin()[i];
            int v = arr[i];
            EXPECT_EQ(u, v);
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
    }

    TEST(Circular_array, Copy_assignment) {
        aul::Circular_array<int> arr0{};
        aul::Circular_array<int> arr1{0, 1, 2, 3, 4, 5, 6, 7};
        arr0 = arr1;

        EXPECT_EQ(arr1.get_allocator(), std::allocator<float>{});
        EXPECT_EQ(arr1.size(), 8);
        EXPECT_GE(arr1.capacity(), 8);
        EXPECT_FALSE(arr1.empty());

        for (std::size_t i = 0; i != arr1.size(); ++i) {
            EXPECT_EQ(arr1[i], i);
        }
    }

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

    TEST(Circular_array, Swap) {
        aul::Circular_array<int> arr0{{0, 2, 4, 6, 8}};
        aul::Circular_array<int> arr1{{1, 3, 5}};

        arr0.swap(arr1);

        EXPECT_EQ(arr0.size(), 3);
        EXPECT_GE(arr0.capacity(), 3);
        EXPECT_EQ(arr0.end() - arr0.begin(), 3);

        EXPECT_EQ(arr1.size(), 5);
        EXPECT_GE(arr1.capacity(), 5);
        EXPECT_EQ(arr1.end() - arr1.begin(), 5);
    }

    //=====================================================
    // Element addition
    //=====================================================

    TEST(Circular_array, Emplace_front_single) {
        Circular_array<int> arr{};

        arr.emplace_front(128);

        EXPECT_EQ(arr[0], 128);
        EXPECT_EQ(arr.size(), 1);
        EXPECT_GE(arr.capacity(), 1);
        EXPECT_EQ(arr.end() - arr.begin(), 1);
        EXPECT_FALSE(arr.empty());
        EXPECT_EQ(*arr.begin(), 128);
    }

    TEST(Circular_array, Emplace_front_multiple) {
        Circular_array<int> arr{};

        constexpr int iterations = 4;

        for (int i = 0; i < iterations; ++i) {
            arr.emplace_front(iterations - i);

            EXPECT_EQ(arr.size(), i + 1);
            EXPECT_GE(arr.capacity(), i + 1);
            EXPECT_EQ(arr.end() - arr.begin(), i + 1);
            EXPECT_FALSE(arr.empty());

            for (int j = 0; j <= i; ++j) {
                int u = arr[j];
                int v = j + (iterations - i);
                EXPECT_EQ(u, v);
            }
        }
    }

    TEST(Circular_array, Emplace_back) {
        Circular_array<int> arr{};

        constexpr int iterations = 1024;

        for (int i = 0; i < iterations; ++i) {
            arr.emplace_back(i);

            EXPECT_EQ(arr.size(), i + 1);
            EXPECT_GE(arr.capacity(), i + 1);
            EXPECT_EQ(arr.end() - arr.begin(), i + 1);
            EXPECT_FALSE(arr.empty());

            for (int j = 0; j <= i; ++j) {
                int u = arr[j];
                int v = j;
                EXPECT_EQ(u, v);
            }
        }
    }

    TEST(Circular_array, Emplace_at_begin) {
        Circular_array<int> arr{};

        constexpr int iterations = 1024;

        for (int i = 0; i < iterations; ++i) {
            arr.emplace(arr.begin(), i);

            EXPECT_EQ(arr.size(), i + 1);
            EXPECT_GE(arr.capacity(), i + 1);
            EXPECT_EQ(arr.end() - arr.begin(), i + 1);
            EXPECT_FALSE(arr.empty());

            for (int j = 0; j <= i; ++j) {
                int u = arr[j];
                int v = i - j;
                EXPECT_EQ(u, v);
            }
        }
    }

    TEST(Circular_array, Emplace_at_middle) {
        std::vector<int> vec{};
        Circular_array<int> arr{};

        constexpr int iterations = 1024;

        for (int i = 0; i < iterations; ++i) {
            auto it1 = vec.begin() + (vec.size() / 2);
            vec.insert(it1, i);

            auto it0 = arr.begin() + (arr.size() / 2);
            arr.emplace(it0, i);

            EXPECT_EQ(arr.size(), i + 1);
            EXPECT_GE(arr.capacity(), i + 1);
            EXPECT_EQ(arr.end() - arr.begin(), i + 1);
            EXPECT_FALSE(arr.empty());
            for (int j = 0; j < arr.size(); ++j) {
                auto u = arr[j];
                auto v = vec[j];
                EXPECT_EQ(u, v);
            }
        }
    }

    TEST(Circular_array, Emplace_at_end) {
        Circular_array<int> arr{};

        constexpr int iterations = 1024;

        for (int i = 0; i < iterations; ++i) {
            arr.emplace(arr.end(), i);

            EXPECT_EQ(arr.size(), i + 1);
            EXPECT_GE(arr.capacity(), i + 1);
            EXPECT_EQ(arr.end() - arr.begin(), i + 1);
            EXPECT_FALSE(arr.empty());

            for (int j = 0; j <= i; ++j) {
                int u = arr[j];
                int v = j;
                EXPECT_EQ(u, v);
            }
        }
    }

    TEST(Circular_array, Insert_n_single) {
        Circular_array<float> arr{};
        arr.insert(arr.begin(), 1, 5.0f);

        EXPECT_EQ(arr.size(), 1);
        EXPECT_GE(arr.capacity(), 1);
        EXPECT_EQ(arr.end() - arr.begin(), 1);
        EXPECT_EQ(arr.cend() - arr.cbegin(), 1);
        EXPECT_EQ(arr[0], 5.0f);
        EXPECT_EQ(arr.begin()[0], 5.0f);
    }

    TEST(Circular_array, Insert_n_multiple) {
        for (int i = 1; i < 1024; i += i) {
            Circular_array<float> arr{};
            arr.insert(arr.begin(), i, 5.0f);

            EXPECT_EQ(arr.size(), i);
            EXPECT_GE(arr.capacity(), i);
            EXPECT_EQ(arr.end() - arr.begin(), i);
            EXPECT_EQ(arr.cend() - arr.cbegin(), i);

            for (int j = 0; j < arr.size(); ++j) {
                EXPECT_EQ(arr[j], 5.0f);
            }
        }
    }

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
        std::initializer_list<int> list{0, 1, 2, 4, 8, 16, 32, 64};
        aul::Circular_array<int> arr{list};

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

    TEST(Circular_array, Erase_front) {
        aul::Circular_array<int> arr{0, 1, 2, 4, 8, 16, 32, 64};

        arr.erase(arr.begin());

        EXPECT_EQ(arr[0], 1);
        EXPECT_EQ(arr.size(), 7);
        EXPECT_GE(arr.capacity(), 7);
        EXPECT_EQ(arr.end() - arr.begin(), 7);
        EXPECT_EQ(arr.back(), 64);
        EXPECT_FALSE(arr.empty());
    }

    TEST(Circular_array, Erase_back) {
        aul::Circular_array<int> arr{0, 1, 2, 4, 8, 16, 32, 64};

        arr.erase(arr.end() - 1);

        EXPECT_EQ(arr[6], 32);
        EXPECT_EQ(arr.size(), 7);
        EXPECT_GE(arr.capacity(), 7);
        EXPECT_EQ(arr.end() - arr.begin(), 7);
        EXPECT_EQ(arr.front(), 0);
        EXPECT_EQ(arr.back(), 32);
        EXPECT_FALSE(arr.empty());

        EXPECT_ANY_THROW(arr.at(7));
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
        EXPECT_TRUE(arr.empty());
        EXPECT_EQ(arr.begin(), arr.end());

        EXPECT_ANY_THROW(arr.at(0));
        EXPECT_ANY_THROW(arr.at(1));
        EXPECT_ANY_THROW(arr.at(2));
    }

    //=====================================================
    // Integration
    //=====================================================

    TEST(Circular_array, Alternating_pop_fronts_and_pop_backs) {
        aul::Circular_array<std::vector<int>> arr{};

        arr.emplace_back(std::vector<int>{1, 2, 3, 4});
        arr.emplace_back(std::vector<int>{1, 2, 3, 4});
        arr.pop_front();
        arr.pop_front();
        arr.emplace_back(std::vector<int>{1, 2, 3, 4});
        arr.pop_front();
        arr.emplace_back(std::vector<int>{1, 2, 3, 4});
        arr.emplace_back(std::vector<int>{1, 2, 3, 4});
        arr.pop_front();
        arr.emplace_back(std::vector<int>{1, 2, 3, 4});
        arr.pop_front();
        arr.pop_front();

        //EXPECT_EQ(arr.size(), 0);
        //EXPECT_EQ(arr.end() - arr.begin(), 0);
        //EXPECT_TRUE(arr.empty());
    }

}

#endif //AUL_CIRCULAR_ARRAY_TESTS_HPP

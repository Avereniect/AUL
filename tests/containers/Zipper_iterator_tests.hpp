//
// Created by avereniect on 1/9/22.
//

#ifndef AUL_ZIPPERATOR_TESTS_HPP
#define AUL_ZIPPERATOR_TESTS_HPP

#include <gtest/gtest.h>

#include <aul/containers/Zipper_iterator.hpp>

namespace aul {

    using int_it = int*;
    using float_it = float*;

    using int_float_fzipit = Forward_zipperator<int*, float*>;
    using int_float_bzipit = Bidirectional_zipperator<int*, float*>;
    using int_float_rzipit = Random_access_zipperator<int*, float*>;

    //=====================================================
    // Static tests
    //=====================================================

    static_assert(std::is_same<std::tuple<int, float>, int_float_fzipit::value_type>::value, "");
    static_assert(std::is_same<std::tuple<int, float>, int_float_bzipit::value_type>::value, "");
    static_assert(std::is_same<std::tuple<int, float>, int_float_rzipit::value_type>::value, "");

    //=====================================================
    // Text fixtures
    //=====================================================


    class Zipperator_fixture : public ::testing::Test {
    protected:

        void SetUp() override {

        }

        std::array<int, 4> int_arr{4, 5, 6, 7};
        std::array<float, 4> float_arr{4.0f, 5.0f, 6.0f, 7.0f};

    };

    TEST_F(Zipperator_fixture, Forward_vector_constructor) {
        int_float_fzipit zip{int_arr.data(), float_arr.data()};

        auto [a, b] = *zip;

        EXPECT_EQ(a, 4);
        EXPECT_EQ(b, 4.0f);
    }

    TEST_F(Zipperator_fixture, Forward_vector_incrementation) {
        int_float_fzipit zip{int_arr.data(), float_arr.data()};
        ++zip;

        auto [a, b] = *zip;

        EXPECT_EQ(a, 5);
        EXPECT_EQ(b, 5.0f);
    }

    TEST_F(Zipperator_fixture, Forward_vector_equality_comparison) {
        int_float_fzipit zip0{int_arr.data(), float_arr.data()};
        ++zip0;
        int_float_fzipit zip1{int_arr.data() + 1, float_arr.data() + 1};

        EXPECT_EQ(zip0, zip1);
    }

    TEST_F(Zipperator_fixture, Forward_vector_inequality_comparison) {
        int_float_fzipit zip0{int_arr.data(), float_arr.data()};
        int_float_fzipit zip1{int_arr.data() + 4, float_arr.data()};

        EXPECT_NE(zip0, zip1);
    }

    TEST_F(Zipperator_fixture, Random_access_reference_assignment) {
        std::array<int, 4> arr0{4, 4, 4, 4};
        std::array<int, 4> arr1{2, 2, 2, 2};

        Random_access_zipperator<int*, int*> it(arr0.data(), arr1.data());

        auto [a, b] = *it;
        a = 0;
        b = 0;

        EXPECT_EQ(arr0[0], 0);
        EXPECT_EQ(arr1[0], 0);
    }

}

#endif //AUL_ZIPPERATOR_TESTS_HPP

//
// Created by avereniect on 6/9/20.
//

#ifndef AUL_MATH_TESTS_HPP
#define AUL_MATH_TESTS_HPP

#include "Math.hpp"

#include <gtest/gtest.h>

#include <numeric>
#include <functional>
#include <random>
#include <iostream>
#include <fstream>

namespace aul::tests {

    TEST(Math, Normalize_int) {
        float x8 = aul::normalize_int<float, std::uint8_t>(0);
        float y8 = aul::normalize_int<float, std::uint8_t>(0xFF);
        EXPECT_EQ(x8, 0.0f);
        EXPECT_EQ(y8, 1.0f);

        float x16 = aul::normalize_int<float, std::uint16_t>(0);
        float y16 = aul::normalize_int<float, std::uint16_t>(0xFFFF);
        EXPECT_EQ(x16, 0.0f);
        EXPECT_EQ(y16, 1.0f);

        float x32 = aul::normalize_int<float, std::uint32_t>(0);
        float y32 = aul::normalize_int<float, std::uint32_t>(0xFFFFFFFF);
        EXPECT_EQ(x32, 0.0f);
        EXPECT_EQ(y32, 1.0f);

        float x64 = aul::normalize_int<float, std::uint64_t>(0);
        float y64 = aul::normalize_int<float, std::uint64_t>(0xFFFFFFFFFFFFFFFF);
        EXPECT_EQ(x64, 0.0f);
        EXPECT_EQ(y64, 1.0f);
    }

    TEST(Math, Float_hashing) {
        const int samples = 1024 * 16;
        std::vector<double> x;
        x.reserve(samples);
        std::vector<double> y;
        y.reserve(samples);

        for (int i = 0; i < samples; ++i) {
            y.push_back(aul::normalize_int<double, std::uint32_t>(byte_hash32(i)));
            x.push_back(i);
        }

        double slope = least_squares_regression(x, y);

        EXPECT_LE(slope, .125   );
    }

}

#endif //AUL_MATH_TESTS_HPP

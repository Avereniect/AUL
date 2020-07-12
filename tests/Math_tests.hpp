//
// Created by avereniect on 6/9/20.
//

#ifndef AUL_MATH_TESTS_HPP
#define AUL_MATH_TESTS_HPP

#include "aul/Math.hpp"

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

        double x_sum = 0;
        for (int i = 0; i < samples; ++i) {
            x_sum += x[i];
        }

        double x2_sum = 0;
        for (int i = 0; i < samples; ++i) {
            x2_sum += x[i] * x[i];
        }

        double y_sum = 0;
        for (int i = 0; i < samples; ++i) {
            y_sum += y[i];
        }

        double xy_sum = 0;
        for (int i = 0; i < samples; ++i) {
            xy_sum += x[i] * y[i];
        }

        std::ofstream foutx{"x.txt"};
        foutx << std::fixed;
        for (auto t : x) {
            foutx << t << "\n";
        }
        foutx.close();

        std::ofstream fouty{"y.txt"};
        fouty << std::fixed;
        for (auto t : y) {
            fouty << t << "\n";
        }
        fouty.close();

        double numerator = samples * xy_sum - x_sum * y_sum;
        double denominator = samples * x2_sum - x_sum * x_sum;

        double slope = numerator / denominator;
        EXPECT_LE(slope, .125   );
    }

}

#endif //AUL_MATH_TESTS_HPP

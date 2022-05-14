//
// Created by avereniect on 3/22/22.
//

#ifndef AUL_MULTISPAN_TESTS_HPP
#define AUL_MULTISPAN_TESTS_HPP

#include <aul/containers/Multispan.hpp>

#include <gtest/gtest.h>

namespace aul::test {

    //=====================================================
    // Span tests
    //=====================================================

    TEST(Span_tests, Constructors) {
        std::array<std::uint32_t, 12> array{};
        std::uint32_t raw_array[12]{};

        aul::Span<std::uint32_t> span0{};
        aul::Span<std::uint32_t> span1{span0};
        aul::Span<std::uint32_t> span2{std::move(span0)};

        aul::Span<std::uint32_t> span3{array.data(), array.size()};
        aul::Span<std::uint32_t> span4{array.size(), array.data()};
        aul::Span<std::uint32_t> span5{array.begin(), array.end()};
        aul::Span<std::uint32_t> span6{array};
        aul::Span<std::uint32_t> span7{raw_array};
    }

    TEST(Span_tests, Constructors_from_const) {
        const std::array<std::uint32_t, 12> array{};
        std::array<const std::uint32_t, 12> array1{};
        const std::uint32_t raw_array[12]{};

        aul::Span<const std::uint32_t> span3{array.data(), array.size()};
        aul::Span<const std::uint32_t> span4{array.begin(), array.end()};
        aul::Span<const std::uint32_t> span5{array};
        aul::Span<const std::uint32_t> span6{raw_array};
        aul::Span<const std::uint32_t> span7{array1};
    }

    //=====================================================
    // Multispan tests
    //=====================================================

    TEST(Multispan_tests, Pair_default_constructor) {
        aul::Multispan<int, float> span{};

        EXPECT_EQ(span.size(), 0);
        EXPECT_TRUE(span.empty());
        EXPECT_EQ(span.begin(), span.end());
    }

    TEST(Multispan_tests, Vector_default_constructor) {
        aul::Multispan<int, float, char> span{};

        EXPECT_EQ(span.size(), 0);
        EXPECT_TRUE(span.empty());
        EXPECT_EQ(span.begin(), span.end());
    }

    TEST(Multispan_tests, Pair_iterator_constructor) {
        std::array<int, 4> arr0{1, 2, 3, 4};
        std::array<float, 4> arr1{1.0f, 2.0f, 3.0f, 4.0f};

        aul::Multispan<int, float> span{arr0.size(), arr0.data(), arr1.data()};

        EXPECT_FALSE(span.empty());
        EXPECT_EQ(span.size(), arr1.size());
        EXPECT_EQ(span.end() - span.begin(), 4);

        {
            auto [a, b] = span.front();

            EXPECT_EQ(a, 1);
            EXPECT_EQ(b, 1.0f);
        }

        {
            auto [a, b] = span.back();

            EXPECT_EQ(a, 4);
            EXPECT_EQ(b, 4.0f);
        }

        for (int i = 0; i < arr0.size(); ++i) {
            auto [a, b] = span.begin()[i];
            EXPECT_EQ(a, i + 1);
            EXPECT_EQ(b, float(i + 1));
        }
    }

    TEST(Multispan_tests, Vector_iterator_constructor) {
        std::array<int, 4> arr0{1, 2, 3, 4};
        std::array<float, 4> arr1{1.0f, 2.0f, 3.0f, 4.0f};
        std::array<char, 4> arr2{'a', 'b', 'c', 'd'};

        aul::Multispan<int, float, char> span{
            arr0.size(),
            arr0.data(),
            arr1.data(),
            arr2.data()
        };

        EXPECT_FALSE(span.empty());
        EXPECT_EQ(span.size(), arr1.size());
        EXPECT_EQ(span.end() - span.begin(), 4);

        {
            auto [a, b, c] = span.front();

            EXPECT_EQ(a, 1);
            EXPECT_EQ(b, 1.0f);
            EXPECT_EQ(c, 'a');
        }

        {
            auto [a, b, c] = span.back();

            EXPECT_EQ(a, 4);
            EXPECT_EQ(b, 4.0f);
            EXPECT_EQ(c, 'd');
        }

        for (int i = 0; i < arr0.size(); ++i) {
            auto [x, y, z] = span.begin()[i];
            EXPECT_EQ(x, i + 1);
            EXPECT_EQ(y, float(i + 1));
            EXPECT_EQ(z, i + 'a');
        }
    }

    TEST(Multispan_tests, Ranged_based_reading) {
        std::array<int, 4> arr0{1, 2, 3, 4};
        std::array<float, 4> arr1{1.0f, 2.0f, 3.0f, 4.0f};

        aul::Multispan<int, float> span{arr0.size(), arr0.data(), arr1.data()};

        int i = 1;
        for (auto [a, b] : span) {
            EXPECT_EQ(a, i);
            EXPECT_EQ(b, float(i));
            ++i;
        }
    }

    TEST(Multispan_tests, Ranged_based_assignment) {
        std::array<int, 4> arr0{1, 2, 3, 4};
        std::array<float, 4> arr1{1.0f, 2.0f, 3.0f, 4.0f};

        aul::Multispan<int, float> span{arr0.size(), arr0.data(), arr1.data()};

        for (auto [a, b] : span) {
            a = 0;
            b = 0.0f;
        }

        for (auto [a, b] : span) {
            EXPECT_EQ(a, 0);
            EXPECT_EQ(b, 0.0f);
        }
    }

    //=====================================================
    // Fixed_multispan tests
    //=====================================================

    TEST(Fixed_multispan_tests, Scalar_default_constructor) {

    }

}

#endif //AUL_MULTISPAN_TESTS_HPP

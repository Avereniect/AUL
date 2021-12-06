//
// Created by avereniect on 9/26/20.
//

#ifndef AUL_MATRIX_TESTS_HPP
#define AUL_MATRIX_TESTS_HPP

#include <numeric>

#include <aul/containers/Matrix.hpp>

#include <gtest/gtest.h>

namespace aul::tests {

    TEST(Matrix, Default_constructor) {
        aul::Matrix<int, 2> mat0;
        aul::Matrix<int, 2> mat1{};

        std::array<std::size_t, 2> dimensions{};

        EXPECT_EQ(mat0.begin(), mat1.end());
        EXPECT_TRUE(mat0.empty());
        EXPECT_EQ(mat0.size(), 0);
        EXPECT_TRUE(std::equal(mat0.begin(), mat0.end(), dimensions.begin()));

        EXPECT_EQ(mat1.begin(), mat1.end());
        EXPECT_TRUE(mat1.empty());
        EXPECT_EQ(mat1.size(), 0);
        EXPECT_TRUE(std::equal(mat1.begin(), mat1.end(), dimensions.begin()));
    }

    TEST(Matrix, Constuctor_with_dimensions) {
        aul::Matrix<int, 1> mat0{{1}};
        aul::Matrix<int, 2> mat1{{1, 2}};
        aul::Matrix<int, 3> mat2{{1, 2, 3}};

        EXPECT_EQ(mat0.begin() + 1, mat0.end());
        EXPECT_FALSE(mat0.empty());
        EXPECT_EQ(mat0.size(), 1);

        EXPECT_EQ(mat1.begin() + 2, mat1.end());
        EXPECT_FALSE(mat1.empty());
        EXPECT_EQ(mat1.size(), 2);

        EXPECT_EQ(mat2.begin() +  6, mat2.end());
        EXPECT_FALSE(mat2.empty());
        EXPECT_EQ(mat2.size(), 6);
    }

    TEST(Matrix, Constuctor_with_dimensions_and_allocator) {
        std::allocator<int> alloc;

        aul::Matrix<int, 1> mat0{{1}, alloc};
    }

    TEST(Matrix, Subscript_operator) {
        aul::Matrix<int, 1> mat0{{4}};

        EXPECT_EQ(mat0[0], 0);
        EXPECT_EQ(mat0[1], 0);
        EXPECT_EQ(mat0[2], 0);
        EXPECT_EQ(mat0[3], 0);


        aul::Matrix<int, 2> mat1{{2, 4}};

        mat1[0][0] = 1;
        mat1[0][1] = 2;
        mat1[1][0] = 3;
        mat1[1][1] = 4;

        EXPECT_EQ(mat1[0][0], 1);
        EXPECT_EQ(mat1[0][1], 2);
        EXPECT_EQ(mat1[1][0], 3);
        EXPECT_EQ(mat1[1][1], 4);
    }

    TEST(Matrix, At) {
        aul::Matrix<int, 2> mat{{4, 4}};
        mat.at({2, 2});

        EXPECT_EQ(mat.at({0, 0}), 0);
        EXPECT_EQ(mat.at({0, 1}), 0);
        EXPECT_EQ(mat.at({0, 2}), 0);
        EXPECT_EQ(mat.at({0, 3}), 0);

        EXPECT_EQ(mat.at({1, 0}), 0);
        EXPECT_EQ(mat.at({1, 1}), 0);
        EXPECT_EQ(mat.at({1, 2}), 0);
        EXPECT_EQ(mat.at({1, 3}), 0);

        EXPECT_EQ(mat.at({2, 0}), 0);
        EXPECT_EQ(mat.at({2, 1}), 0);
        EXPECT_EQ(mat.at({2, 2}), 0);
        EXPECT_EQ(mat.at({2, 3}), 0);

        EXPECT_EQ(mat.at({3, 0}), 0);
        EXPECT_EQ(mat.at({3, 1}), 0);
        EXPECT_EQ(mat.at({3, 2}), 0);
        EXPECT_EQ(mat.at({3, 3}), 0);
    }

    TEST(Matrix, Const_at) {
        const aul::Matrix<int, 2> mat{{4, 4}};

        EXPECT_EQ(mat.at({0, 0}), 0);
        EXPECT_EQ(mat.at({0, 1}), 0);
        EXPECT_EQ(mat.at({0, 2}), 0);
        EXPECT_EQ(mat.at({0, 3}), 0);

        EXPECT_EQ(mat.at({1, 0}), 0);
        EXPECT_EQ(mat.at({1, 1}), 0);
        EXPECT_EQ(mat.at({1, 2}), 0);
        EXPECT_EQ(mat.at({1, 3}), 0);

        EXPECT_EQ(mat.at({2, 0}), 0);
        EXPECT_EQ(mat.at({2, 1}), 0);
        EXPECT_EQ(mat.at({2, 2}), 0);
        EXPECT_EQ(mat.at({2, 3}), 0);

        EXPECT_EQ(mat.at({3, 0}), 0);
        EXPECT_EQ(mat.at({3, 1}), 0);
        EXPECT_EQ(mat.at({3, 2}), 0);
        EXPECT_EQ(mat.at({3, 3}), 0);
    }

    TEST(Matrix, clear) {
        aul::Matrix<int, 4> mat{{4, 4, 4, 4}};

        mat.clear();

        EXPECT_EQ(mat.size(), 0);

        auto dims = mat.dimensions();
        EXPECT_EQ(dims[0], 0);
        EXPECT_EQ(dims[1], 0);
        EXPECT_EQ(dims[2], 0);
        EXPECT_EQ(dims[3], 0);

        EXPECT_ANY_THROW(mat.at({0, 0, 0, 0}));
    }

    TEST(Matrix, resize_from_empty) {
        aul::Matrix<int, 2> mat0{};
        mat0.resize({ 0, 0 }, 0);

        auto d0 = mat0.dimensions();
        EXPECT_EQ(d0[0], 0);
        EXPECT_EQ(d0[1], 0);
    }

    TEST(Matrix, resize_to_empty_dimensions) {
        aul::Matrix<int, 2> mat1{};
        mat1.resize({4, 0}, 1);

        auto d1 = mat1.dimensions();
        EXPECT_EQ(d1[0], 0);
        EXPECT_EQ(d1[1], 0);
        EXPECT_EQ(mat1.data(), nullptr);
    }

    TEST(Matrix, resize_increase_one_dimension_from_empty) {
        aul::Matrix<int, 2> mat3{};
        mat3.resize({1, 4}, 0xFF);
        auto d3 = mat3.dimensions();
        EXPECT_EQ(d3[0], 1);
        EXPECT_EQ(d3[1], 4);
        EXPECT_NE(mat3.data(), nullptr);

        EXPECT_EQ(mat3[0][0], 0xFF);
        EXPECT_EQ(mat3[0][1], 0xFF);
        EXPECT_EQ(mat3[0][2], 0xFF);
        EXPECT_EQ(mat3[0][3], 0xFF);
    }

    TEST(Matrix, reisze_increase) {
        aul::Matrix<int, 2> mat4{};
        mat4.resize({1, 2}, 0xFF);
        mat4.resize({1, 4}, 0x55);
        auto d4 = mat4.dimensions();
        EXPECT_EQ(d4[0], 1);
        EXPECT_EQ(d4[1], 4);
        EXPECT_NE(mat4.data(), nullptr);

        EXPECT_EQ(mat4[0][0], 0xFF);
        EXPECT_EQ(mat4[0][1], 0xFF);
        EXPECT_EQ(mat4[0][2], 0x55);
        EXPECT_EQ(mat4[0][3], 0x55);
    }

    TEST(Matrix, resize_increase) {
        aul::Matrix<int, 2> mat5{};
        mat5.resize({1, 1}, 0xFF);
        mat5.resize({2, 2}, 0x55);
        auto d5 = mat5.dimensions();
        EXPECT_EQ(d5[0], 2);
        EXPECT_EQ(d5[1], 2);
        EXPECT_NE(mat5.data(), nullptr);

        EXPECT_EQ(mat5[0][0], 0xFF);
        EXPECT_EQ(mat5[0][1], 0x55);
        EXPECT_EQ(mat5[1][0], 0x55);
        EXPECT_EQ(mat5[1][1], 0x55);
    }

    TEST(Matrix, resize_decrease_one_dimension) {
        aul::Matrix<int, 2> mat{{1, 4}, 0xFF};
        mat.resize({1, 1}, 0x55);

        auto d = mat.dimensions();
        EXPECT_EQ(d[0], 1);
        EXPECT_EQ(d[1], 1);
        EXPECT_NE(mat.data(), nullptr);

        EXPECT_EQ(mat[0][0], 0xFF);
    }

    TEST(Matrix, resize_decrease_two_dimensionss1) {
        aul::Matrix<int, 2> mat{{2, 2}, 0xFF};
        mat.resize({1, 1}, 0x55);

        auto d = mat.dimensions();
        EXPECT_EQ(d[0], 1);
        EXPECT_EQ(d[1], 1);
        EXPECT_NE(mat.data(), nullptr);

        EXPECT_EQ(mat[0][0], 0xFF);
    }

    TEST(Matrix, resize_decrease_two_dimensionss2) {
        aul::Matrix<int, 3> mat{{3, 3, 3}, 0x00};
        std::iota(mat.data(), mat.data() + mat.size(), 0);

        mat.resize({2, 2, 2}, 0x55);

        auto d = mat.dimensions();
        EXPECT_EQ(d[0], 2);
        EXPECT_EQ(d[1], 2);
        EXPECT_NE(mat.data(), nullptr);

        EXPECT_EQ(mat[0][0][0], 0x00);
        EXPECT_EQ(mat[0][0][1], 0x01);
        EXPECT_EQ(mat[0][1][0], 0x03);
        EXPECT_EQ(mat[0][1][1], 0x04);

        EXPECT_EQ(mat[1][0][0], 0x09);
        EXPECT_EQ(mat[1][0][1], 0x0A);
        EXPECT_EQ(mat[1][1][0], 0x0C);
        EXPECT_EQ(mat[1][1][1], 0x0D);
    }

}

#endif //AUL_MATRIX_TESTS_HPP

//
// Created by avereniect on 9/26/20.
//

#ifndef AUL_MATRIX_TESTS_HPP
#define AUL_MATRIX_TESTS_HPP

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

}

#endif //AUL_MATRIX_TESTS_HPP

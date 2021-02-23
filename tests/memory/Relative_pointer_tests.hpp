//
// Created by avereniect on 7/20/20.
//

#ifndef AUL_VIRTUAL_ALLOCATOR_TESTS_HPP
#define AUL_VIRTUAL_ALLOCATOR_TESTS_HPP

#include <type_traits>

//#define AUL_INTEGER_POINTERS
#include <aul/memory/Relative_pointer.hpp>

#include <gtest/gtest.h>

namespace aul::tests {

    static_assert(sizeof(float) >= 4);

    using float_ptr8_1 = Relative_pointer<float, std::int8_t, 1>;
    using float_ptr8_2 = Relative_pointer<float, std::int8_t, 2>;
    using float_ptr8_4 = Relative_pointer<float, std::int8_t, 4>;

    using double_ptr8_1 = Relative_pointer<double, std::int8_t, 1>;
    using double_ptr8_2 = Relative_pointer<double, std::int8_t, 2>;
    using double_ptr8_4 = Relative_pointer<double, std::int8_t, 4>;
    using double_ptr8_8 = Relative_pointer<double, std::int8_t, 8>;


    using float_ptr16 = Relative_pointer<float, std::int16_t>;
    using const_float_ptr16 = Const_relative_pointer<float, std::int16_t>;
    using uint32_ptr16 = Relative_pointer<std::uint32_t, std::int16_t>;

    //=====================================================
    // Type traits
    //=====================================================

    static_assert(std::is_same_v<std::pointer_traits<float_ptr16>::pointer, float_ptr16>);
    static_assert(std::is_same_v<std::pointer_traits<float_ptr16>::element_type, float>);
    static_assert(std::is_same_v<std::pointer_traits<float_ptr16>::rebind<std::uint32_t>, uint32_ptr16>);

    //=====================================================
    // Constructor tests
    //=====================================================

    TEST(Relative_pointer, Construct_from_nullptr) {
        float_ptr16 ptr0 = nullptr;
        const_float_ptr16 ptr1 = nullptr;

        EXPECT_EQ(ptr0, nullptr);
        EXPECT_EQ(ptr1, nullptr);
        EXPECT_EQ(ptr0, ptr1);
    }

    TEST(Relative_pointer, Construct_from_raw_pointer) {
        alignas(alignof(float)) unsigned char buffer[64];

        auto floats = reinterpret_cast<float*>(buffer);
        floats[0] = 1.0f;
        floats[1] = 2.0f;
        floats[2] = 4.0f;
        floats[3] = 8.0f;

        float_ptr16& ptr0 = *new(buffer + 4 * sizeof(float) + 0 * sizeof(float_ptr16)) float_ptr16(floats);

        EXPECT_EQ(ptr0[0], 1.0f);
        EXPECT_EQ(ptr0[1], 2.0f);
        EXPECT_EQ(ptr0[2], 4.0f);
        EXPECT_EQ(ptr0[3], 8.0f);
    }

    TEST(Relative_pointer, Stride_2_align_4) {
        alignas(alignof(float)) unsigned char buffer[128];
        float* lower_floats = reinterpret_cast<float*>(buffer);
        lower_floats[0] = 1.0f;
        lower_floats[1] = 2.0f;
        lower_floats[2] = 4.0f;
        lower_floats[3] = 8.0f;

        float_ptr8_2* pointers = reinterpret_cast<float_ptr8_2*>(buffer + 4 * sizeof(float));

        float* upper_floats = reinterpret_cast<float*>(buffer + 4 * sizeof (float_ptr8_2) + 4 * sizeof(float));
        upper_floats[0] = 16.0f;
        upper_floats[1] = 32.0f;
        upper_floats[2] = 64.0f;
        upper_floats[3] = 128.0f;

        float* p = lower_floats + 3;
        float_ptr8_4& ptr0 = *new(pointers + 0) float_ptr8_4(p);
        float_ptr8_4& ptr1 = *new(pointers + 1) float_ptr8_4(p);
        float_ptr8_4& ptr2 = *new(pointers + 2) float_ptr8_4(p);
        float_ptr8_4& ptr3 = *new(pointers + 3) float_ptr8_4(p);

        EXPECT_EQ(*ptr0, *p);
        EXPECT_EQ(*ptr1, *p);
        EXPECT_EQ(*ptr2, *p);
        EXPECT_EQ(*ptr3, *p);
    }

    TEST(Relative_pointer, Stride_4_align_4) {
        alignas(alignof(float)) unsigned char buffer[128];
        float* lower_floats = reinterpret_cast<float*>(buffer);
        lower_floats[0] = 1.0f;
        lower_floats[1] = 2.0f;
        lower_floats[2] = 4.0f;
        lower_floats[3] = 8.0f;

        float_ptr8_2* pointers = reinterpret_cast<float_ptr8_2*>(buffer + 4 * sizeof(float));

        float* upper_floats = reinterpret_cast<float*>(buffer + 4 * sizeof (float_ptr8_2) + 4 * sizeof(float));
        upper_floats[0] = 16.0f;
        upper_floats[1] = 32.0f;
        upper_floats[2] = 64.0f;
        upper_floats[3] = 128.0f;

        float* p = lower_floats + 3;
        float_ptr8_4& ptr0 = *new(pointers + 0) float_ptr8_4(p);
        float_ptr8_4& ptr1 = *new(pointers + 1) float_ptr8_4(p);
        float_ptr8_4& ptr2 = *new(pointers + 2) float_ptr8_4(p);
        float_ptr8_4& ptr3 = *new(pointers + 3) float_ptr8_4(p);

        EXPECT_EQ(*ptr0, *p);
        EXPECT_EQ(*ptr1, *p);
        EXPECT_EQ(*ptr2, *p);
        EXPECT_EQ(*ptr3, *p);
    }

    TEST(Relative_pointer, Stride_2_align_8) {
        alignas(alignof(double)) unsigned char buffer[128];
        double* lower_doubles = reinterpret_cast<double*>(buffer);
        lower_doubles[0] = 1.0;
        lower_doubles[1] = 2.0;

        double_ptr8_2 * pointers = reinterpret_cast<double_ptr8_2*>(buffer + 4 * sizeof(float));

        double* upper_doubles = reinterpret_cast<double*>(buffer + 8 * sizeof (double_ptr8_2) + 2 * sizeof(double));
        upper_doubles[0] = 4.0;
        upper_doubles[1] = 8.0;

        double* p = upper_doubles + 0;
        double_ptr8_2& ptr0 = *new(pointers + 0) double_ptr8_2(p);
        double_ptr8_2& ptr1 = *new(pointers + 1) double_ptr8_2(p);
        double_ptr8_2& ptr2 = *new(pointers + 2) double_ptr8_2(p);
        double_ptr8_2& ptr3 = *new(pointers + 3) double_ptr8_2(p);

        EXPECT_EQ(*ptr0, *p);
        EXPECT_EQ(*ptr1, *p);
        EXPECT_EQ(*ptr2, *p);
        EXPECT_EQ(*ptr3, *p);
    }

    TEST(Relative_pointer, Copy_construct_nullptr) {
        float_ptr16 ptr0 = nullptr;
        float_ptr16 ptr1 = nullptr;

        EXPECT_EQ(ptr0, nullptr);
        EXPECT_EQ(ptr1, nullptr);
        EXPECT_EQ(ptr0, ptr1);
    }

    TEST(Relative_pointer, Copy_constructor) {
        unsigned char buffer[64];
        auto floats = reinterpret_cast<float*>(buffer);
        floats[0] = 1.0f;
        floats[1] = 2.0f;
        floats[2] = 4.0f;
        floats[3] = 8.0f;

        float_ptr16& ptr0 = *new(buffer + 4 * sizeof(float) + 0 * sizeof(float_ptr16)) float_ptr16(floats);
        float_ptr16& ptr1 = *new(buffer + 4 * sizeof(float) + 1 * sizeof(float_ptr16)) float_ptr16(ptr0);

        EXPECT_EQ(ptr1[0], 1.0f);
        EXPECT_EQ(ptr1[1], 2.0f);
        EXPECT_EQ(ptr1[2], 4.0f);
        EXPECT_EQ(ptr1[3], 8.0f);
    }

    TEST(Relative_pointer, Copy_assignment_operator) {
        unsigned char buffer[64];
        auto floats = reinterpret_cast<float*>(buffer);
        floats[0] = 1.0f;
        floats[1] = 2.0f;
        floats[2] = 4.0f;
        floats[3] = 8.0f;

        float_ptr16& ptr0 = *new(buffer + 4 * sizeof(float) + 0 * sizeof(float_ptr16)) float_ptr16(floats);
        float_ptr16& ptr1 = *new(buffer + 4 * sizeof(float) + 1 * sizeof(float_ptr16)) float_ptr16(nullptr);
        ptr1 = ptr0;

        EXPECT_EQ(static_cast<float*>(ptr0),static_cast<float*>(ptr1));
    }

    TEST(Relative_pointer, Raw_pointer_assignment) {
        unsigned char buffer[64];
        auto floats = reinterpret_cast<float*>(buffer);
        floats[0] = 1.0f;
        floats[1] = 2.0f;
        floats[2] = 4.0f;
        floats[3] = 8.0f;

        float_ptr16& ptr0 = *new(buffer + 4 * sizeof(float)) float_ptr16(nullptr);

        ptr0 = floats + 0;
        EXPECT_EQ(*ptr0, 1.0f);
        ptr0 = floats + 1;
        EXPECT_EQ(*ptr0, 2.0f);
        ptr0 = floats + 2;
        EXPECT_EQ(*ptr0, 4.0f);
        ptr0 = floats + 3;
        EXPECT_EQ(*ptr0, 8.0f);
    }

    TEST(Relative_pointer, Arithmetic_assignment) {
        unsigned char buffer[64];
        auto floats = reinterpret_cast<float*>(buffer);
        floats[0] = 1.0f;
        floats[1] = 2.0f;
        floats[2] = 4.0f;
        floats[3] = 8.0f;
        floats[4] = 16.0f;
        floats[5] = 32.0f;
        floats[6] = 64.0f;
        floats[7] = 128.0f;

        float_ptr16& ptr0 = *new(buffer + 8 * sizeof(float)) float_ptr16(floats);

        EXPECT_EQ(*ptr0, 1.0f);
        ptr0 += 1;
        EXPECT_EQ(*ptr0, 2.0f);
        ptr0 += 2;
        EXPECT_EQ(*ptr0, 8.0f);
        ptr0 -= 3;
        EXPECT_EQ(*ptr0, 1.0f);
        ptr0 += 7;
        EXPECT_EQ(*ptr0, 128.0f);
        ptr0 -= 6;
        EXPECT_EQ(*ptr0, 2.0f);
    }

    TEST(Relative_pointer, Increment_and_decrement_operators) {
        unsigned char buffer[64];
        auto floats = reinterpret_cast<float*>(buffer);
        floats[0] = 1.0f;
        floats[1] = 2.0f;
        floats[2] = 4.0f;
        floats[3] = 8.0f;

        float_ptr16& ptr0 = *new(buffer + 4 * sizeof(float)) float_ptr16(floats);
        EXPECT_EQ(*(ptr0++), 1.0f);
        EXPECT_EQ(*(ptr0), 2.0f);
        EXPECT_EQ(*(++ptr0), 4.0f);

        EXPECT_EQ(*(ptr0--), 4.0f);
        EXPECT_EQ(*(ptr0), 2.0f);
        EXPECT_EQ(*(--ptr0), 1.0f);
    }

    TEST(Relative_pointer, Comparison_operators) {
        unsigned char buffer[64];
        auto floats = reinterpret_cast<float*>(buffer);
        floats[0] = 1.0f;
        floats[1] = 2.0f;
        floats[2] = 4.0f;
        floats[3] = 8.0f;

        float_ptr16& ptr0 = *new(buffer + 4 * sizeof(float) + 0 * sizeof(float_ptr16)) float_ptr16(floats + 0);
        float_ptr16& ptr1 = *new(buffer + 4 * sizeof(float) + 1 * sizeof(float_ptr16)) float_ptr16(floats + 1);
        float_ptr16& ptr2 = *new(buffer + 4 * sizeof(float) + 2 * sizeof(float_ptr16)) float_ptr16(floats + 2);
        float_ptr16& ptr3 = *new(buffer + 4 * sizeof(float) + 3 * sizeof(float_ptr16)) float_ptr16(floats + 3);

        EXPECT_LT(ptr0, ptr1);
        EXPECT_LE(ptr0, ptr0);
        EXPECT_LE(ptr0, ptr1);
        EXPECT_LE(ptr0, ptr1);

        EXPECT_EQ(ptr0, ptr0);
        EXPECT_EQ(ptr1, ptr1);
        EXPECT_EQ(ptr2, ptr2);
        EXPECT_EQ(ptr3, ptr3);

        EXPECT_GT(ptr1, ptr0);
        EXPECT_GE(ptr1, ptr1);
        EXPECT_GE(ptr1, ptr0);

        EXPECT_NE(ptr0, ptr1);
        EXPECT_NE(ptr1, ptr2);
        EXPECT_NE(ptr2, ptr3);
    }

}

#endif //AUL_VIRTUAL_ALLOCATOR_TESTS_HPP

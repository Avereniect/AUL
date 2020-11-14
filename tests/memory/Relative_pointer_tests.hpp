//
// Created by avereniect on 10/17/20.
//

#ifndef AUL_RELATIVE_POINTER_TESTS_HPP
#define AUL_RELATIVE_POINTER_TESTS_HPP

#include <aul/memory/Relative_pointer.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <type_traits>

namespace aul::tests {

    using float_ptr_8 = aul::Relative_pointer<float, std::int8_t>;
    using float_ptr_16 = aul::Relative_pointer<float, std::int16_t>;
    using float_ptr_32 = aul::Relative_pointer<float, std::int32_t>;

    static_assert(!std::is_trivial_v<float_ptr_8>);
    static_assert(!std::is_trivial_v<float_ptr_16>);
    static_assert(!std::is_trivial_v<float_ptr_32>);

    TEST(Relative_pointer, Constructors) {
        constexpr std::size_t buffer_size = sizeof(float) * 32;
        char buffer[buffer_size];

        static_assert(3 * sizeof(float_ptr_8) <= sizeof(float));
        auto floats = reinterpret_cast<float*>(buffer + sizeof(float));
        std::fill_n(floats, 4, 0.0f);

        //Construct from absolute pointer
        float_ptr_8* ptr_addr0 = reinterpret_cast<float_ptr_8*>(buffer);
        new(ptr_addr0) float_ptr_8{floats};
        float_ptr_8& ptr0 = *(ptr_addr0);

        EXPECT_EQ(floats, static_cast<float*>(ptr0));
        EXPECT_EQ(ptr0[0], 0.0f);
        EXPECT_EQ(ptr0[1], 0.0f);
        EXPECT_EQ(ptr0[2], 0.0f);
        EXPECT_EQ(ptr0[3], 0.0f);

        //Copy construct from first pointer
        auto ptr_addr1 = ptr_addr0 + 1;
        new(ptr_addr1) float_ptr_8 {ptr0};
        float_ptr_8& r_ptr1 = *ptr_addr1;

        EXPECT_EQ(floats, static_cast<float*>(r_ptr1));
        EXPECT_EQ(r_ptr1[0], 0.0f);
        EXPECT_EQ(r_ptr1[1], 0.0f);
        EXPECT_EQ(r_ptr1[2], 0.0f);
        EXPECT_EQ(r_ptr1[3], 0.0f);

        //Move construct from second pointer
        auto ptr_addr2 = ptr_addr1 + 1;
        new(ptr_addr2) float_ptr_8 {std::move(r_ptr1)};
        float_ptr_8& r_ptr2 = *(ptr_addr2);

        EXPECT_EQ(floats, static_cast<float*>(r_ptr2));
        EXPECT_EQ(r_ptr2[0], 0.0f);
        EXPECT_EQ(r_ptr2[1], 0.0f);
        EXPECT_EQ(r_ptr2[2], 0.0f);
        EXPECT_EQ(r_ptr2[3], 0.0f);
    }

    TEST(Relative_pointer, Assignment_operators) {
        char buffer[1024];

        auto ptr_addr = reinterpret_cast<float_ptr_8*>(buffer);

        auto elem_ptr = reinterpret_cast<float*>(ptr_addr + 4 * sizeof(float_ptr_8));

        new(ptr_addr + 0) float_ptr_8{};
        float_ptr_8& ptr0 = ptr_addr[0];

        new(ptr_addr + 1) float_ptr_8{ptr0};
        float_ptr_8& ptr1 = ptr_addr[1];
        ptr1 = ptr0;

        new(ptr_addr + 2) float_ptr_8{};
        float_ptr_8& ptr2 = ptr_addr[2];
        ptr2 = ptr0;

        new(ptr_addr + 3) float_ptr_8{};
        float_ptr_8& ptr3 = ptr_addr[3];
        ptr3 = std::move(ptr2);

        EXPECT_EQ(static_cast<float*>(ptr0), static_cast<float*>(ptr1));
        EXPECT_EQ(static_cast<float*>(ptr0), static_cast<float*>(ptr3));
    }

    TEST(Relative_pointer, Arithmetic) {
        char buffer[1024];

        auto ptr_addr = reinterpret_cast<float_ptr_8*>(buffer);
        auto floats = reinterpret_cast<float*>(buffer + 4 * sizeof(float_ptr_8));


    }

}

#endif //AUL_RELATIVE_POINTER_TESTS_HPP

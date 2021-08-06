#ifndef AUL_BIT_FIELD_ITERATOR_TESTS_HPP
#define AUL_BIT_FIELD_ITERATOR_TESTS_HPP

#include <aul/containers/Bit_field_iterator.hpp>

#include <gtest/gtest.h>

namespace aul::tests {

    //=====================================================
    // Bit field reference tests
    //=====================================================

    TEST(Bit_field_reference, Full_int64) {
        std::uint64_t data[1] = {734};
        Bit_field_ref<std::uint64_t> ref{data, 0, 64};
        std::uint64_t x = ref;
        EXPECT_EQ(x, data[0]);

        ref = 256;
        EXPECT_EQ(ref, 256);
    }

    TEST(Bit_field_reference, Partial_int64) {
        std::uint64_t data[1] = {0xF0F0F0F0};
        Bit_field_ref<std::uint64_t> ref{data, 0, 16};
        std::uint64_t x = ref;
        EXPECT_EQ(x, 0xF0F0);

        ref = 0x11111111;
        EXPECT_EQ(ref, 0x1111);
    }

    TEST(Bit_field_reference, Offset_int64_32) {
        std::uint64_t data[1] = {0x0123456789ABCDEF};
        Bit_field_ref<std::uint64_t> ref0{data, 32, 32};
        std::uint64_t x = ref0;
        EXPECT_EQ(x, 0x01234567);

        Bit_field_ref<std::uint64_t> ref1{data, 16, 32};
        std::uint64_t y = ref1;
        EXPECT_EQ(y, 0x456789AB);
    }

    TEST(Bit_field_reference, Split_uint32_t) {
        std::uint32_t data[2] = {0xCDCD0000, 0x0000EFEF};
        Bit_field_ref<std::uint32_t> ref{data, 16, 32};
        std::uint32_t x = ref;
        EXPECT_EQ(x, 0xEFEFCDCD);
    }

    //=====================================================
    // Bit field iterator tests
    //=====================================================

    TEST(Bit_field_iterators, Dereference_full) {
        std::uint64_t data[2] = {0xFEDCBA9876543210};
        Bit_field_iterator<std::uint64_t> it{data, 0, 64};
        EXPECT_EQ(*it, 0xFEDCBA9876543210l);
    }

    TEST(Bit_field_iterators, Dereference_partial) {
        std::uint64_t data[2] = {0xFEDCBA9876543210};
        Bit_field_iterator<std::uint64_t> it{data, 0, 32};
        EXPECT_EQ(*it, 0x76543210);
    }

    TEST(Bit_field_iterators, Pre_increment_full) {
        std::uint64_t data[2] = {0xFFFFEEEEDDDDCCCC, 0xBBBBAAAA99998888};
        Bit_field_iterator<std::uint64_t> it{data, 0, 64};
        ++it;
        EXPECT_EQ(*it, 0xBBBBAAAA99998888l);
    }

    TEST(Bit_field_iterators, Post_increment_full) {
        std::uint64_t data[2] = {0xFFFFEEEEDDDDCCCC, 0xBBBBAAAA99998888};
        Bit_field_iterator<std::uint64_t> it{data, 0, 64};
        it++;
        EXPECT_EQ(*it, 0xBBBBAAAA99998888l);
    }

    TEST(Bit_field_iterators, Increment_partial) {
        std::uint64_t data[2] = {0xFFFFEEEEDDDDCCCC};
        Bit_field_iterator<std::uint64_t> it{data, 0, 32};
        ++it;
        EXPECT_EQ(*it, 0xFFFFEEEE);
    }

    TEST(Bit_field_iterators, Increment_split) {
        std::uint64_t data[2] = {0xFFFFEEEEDDDDCCCC, 0xBBBBAAAA99998888};
        Bit_field_iterator<std::uint64_t> it{data, 0, 48};
        ++it;
        EXPECT_EQ(*it, 0x99998888FFFF);
    }

    TEST(Bit_field_iterators, Decrement_full) {
        std::uint64_t data[2] = {0xFFFFEEEEDDDDCCCC, 0xBBBBAAAA99998888};
        Bit_field_iterator<std::uint64_t> it{data + 1, 0, 64};
        --it;
        EXPECT_EQ(*it, 0xFFFFEEEEDDDDCCCC);
    }

    TEST(Bit_field_iterators, Decrement_partial) {
        std::uint64_t data[2] = {0xFFFFEEEEDDDDCCCC, 0xBBBBAAAA99998888};
        Bit_field_iterator<std::uint64_t> it{data, 32, 32};
        --it;
        EXPECT_EQ(*it, 0xDDDDCCCC);
    }

    TEST(Bit_field_iterators, Addition_full) {
        std::uint64_t data[2] = {0xFFFFEEEEDDDDCCCC, 0xBBBBAAAA99998888};
        Bit_field_iterator<std::uint64_t> it{data, 0, 64};
        it += 1;
        EXPECT_EQ(*it, 0xBBBBAAAA99998888);
    }

    TEST(Bit_field_iterators, Addition_half) {
        std::uint64_t data[1] = {0xFFFFEEEEDDDDCCCC};
        Bit_field_iterator<std::uint64_t> it{data, 0, 32};
        it += 1;
        EXPECT_EQ(*it, 0xFFFFEEEE);
    }

    TEST(Bit_field_iterators, Addition_48_64) {
        std::uint64_t data[2] = {0xFFFFEEEEDDDDCCCC, 0xBBBBAAAA99998888};
        Bit_field_iterator<std::uint64_t> it{data, 0, 48};
        it += 1;
        EXPECT_EQ(*it, 0x99998888FFFF);
    }

    TEST(Bit_field_iterators, Addition_by_greater_than_1) {
        std::uint64_t data[2] = {0xFFFFEEEEDDDDCCCC, 0xBBBBAAAA99998888};
        Bit_field_iterator<std::uint64_t> it{data, 0, 16};
        it += 2;
        EXPECT_EQ(*it, 0xEEEE);
        it += 2;
        EXPECT_EQ(*it, 0x8888);
        it = {data, 0, 16};
        it += 7;
        EXPECT_EQ(*it, 0xBBBB);
    }

    TEST(Bit_field_iterators, Addition_by_greater_than_8_whole) {
        std::uint64_t data[12] = {
            0xFFFFFFFFFFFFFFFF,
            0xEEEEEEEEEEEEEEEE,
            0xDDDDDDDDDDDDDDDD,
            0xCCCCCCCCCCCCCCCC,
            0xBBBBBBBBBBBBBBBB,
            0xAAAAAAAAAAAAAAAA,
            0x9999999999999999,
            0x8888888888888888,
            0x7777777777777777,
            0x6666666666666666,
            0x5555555555555555,
            0x4444444444444444
        };

        Bit_field_iterator<std::uint64_t> it{data, 0, 64};
        it += 10;
        EXPECT_EQ(*it, 0x5555555555555555l);
    }

    TEST(Bit_field_iterators, subtraction_by_greater_than_8_whole) {
        std::uint64_t data[12] = {
            0xFFFFFFFFFFFFFFFF,
            0xEEEEEEEEEEEEEEEE,
            0xDDDDDDDDDDDDDDDD,
            0xCCCCCCCCCCCCCCCC,
            0xBBBBBBBBBBBBBBBB,
            0xAAAAAAAAAAAAAAAA,
            0x9999999999999999,
            0x8888888888888888,
            0x7777777777777777,
            0x6666666666666666,
            0x5555555555555555,
            0x4444444444444444
        };

        Bit_field_iterator<std::uint64_t> it{data + 11, 0, 64};
        it += -11;
        EXPECT_EQ(*it, 0xFFFFFFFFFFFFFFFFl);
    }

    TEST(Bit_field_iterators, Addition_by_negative_values) {
        std::uint64_t data[2] = {0xFFFFEEEEDDDDCCCC, 0xBBBBAAAA99998888};
        Bit_field_iterator<std::uint64_t> it{data + 1, 0, 64};
        it += -1;
        EXPECT_EQ(*it, 0xFFFFEEEEDDDDCCCC);

        it = {data  + 1, 0, 32};
        it += -2;
        EXPECT_EQ(*it, 0xDDDDCCCC);
    }

}

#endif //AUL_BIT_FIELD_ITERATOR_TESTS_HPP

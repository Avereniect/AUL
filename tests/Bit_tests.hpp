#ifndef AUL_TESTS_BIT_TESTS_HPP
#define AUL_TESTS_BIT_TESTS_HPP

#include <aul/Bits.hpp>

#include <cstdint>

#include <gtest/gtest.h>
#include <iostream>

namespace aul::tests {

    TEST(Bits, Bits_to_string) {
        EXPECT_EQ(aul::bits_to_string<std::uint8_t>(0x00), "00000000");
        EXPECT_EQ(aul::bits_to_string<std::uint8_t>(0xFF), "11111111");
        EXPECT_EQ(aul::bits_to_string<std::uint8_t>(0x0F), "00001111");
        EXPECT_EQ(aul::bits_to_string<std::uint8_t>(0x3F), "00111111");
        EXPECT_EQ(aul::bits_to_string<std::uint8_t>(0xF0), "11110000");
        EXPECT_EQ(aul::bits_to_string<std::uint8_t>(0xF3), "11110011");

        EXPECT_EQ(aul::bits_to_string<std::uint16_t>(0x0000), "0000000000000000");
        EXPECT_EQ(aul::bits_to_string<std::uint16_t>(0xFFFF), "1111111111111111");
        EXPECT_EQ(aul::bits_to_string<std::uint16_t>(0x00F3), "0000000011110011");
        EXPECT_EQ(aul::bits_to_string<std::uint16_t>(0x30F3), "0011000011110011");
        EXPECT_EQ(aul::bits_to_string<std::uint16_t>(0x31F3), "0011000111110011");
        EXPECT_EQ(aul::bits_to_string<std::uint16_t>(0xA513), "1010010100010011");
    }

    TEST(Bits, Mod_pow2) {
        EXPECT_EQ(aul::mod_pow2(1u, 1u), 1);
        EXPECT_EQ(aul::mod_pow2(2u, 1u), 0);
        EXPECT_EQ(aul::mod_pow2(5u, 1u), 1);
    }

}

#endif //AUL_TESTS_BIT_TESTS_HPP

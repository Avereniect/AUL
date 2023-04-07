#ifndef AUL_DRLE_RANGE_TESTS_HPP
#define AUL_DRLE_RANGE_TESTS_HPP

#include <aul/DRLE_range.hpp>

#include <cstdint>
#include <gtest/gtest.h>

namespace aul_tests {

    TEST(DRLE_range, Empty) {
        std::vector<std::uint32_t> data;

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};

        EXPECT_TRUE(compressed_data.empty());
        EXPECT_EQ(compressed_data.size(), 0);
    }

    TEST(DRLE_range, Slope_zero_single_element) {
        std::vector<std::uint32_t> data = {10};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 1);
        EXPECT_EQ(compressed_data[0], 10);
    }

    TEST(DRLE_range, Slope_zero_multiple_elements) {
        std::vector<std::uint32_t> data = {10, 10, 10, 10};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 4);
        EXPECT_EQ(compressed_data[0], 10);
        EXPECT_EQ(compressed_data[1], 10);
        EXPECT_EQ(compressed_data[2], 10);
        EXPECT_EQ(compressed_data[3], 10);
    }

    TEST(DRLE_range, Slope_one) {
        std::vector<std::uint32_t> data = {1, 2, 3, 4};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 4);
        EXPECT_EQ(compressed_data[0], 1);
        EXPECT_EQ(compressed_data[1], 2);
        EXPECT_EQ(compressed_data[2], 3);
        EXPECT_EQ(compressed_data[3], 4);
    }

    TEST(DRLE_range, Slope_negative_one) {
        std::vector<std::uint32_t> data = {4, 3, 2, 1};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 4);
        EXPECT_EQ(compressed_data[0], 4);
        EXPECT_EQ(compressed_data[1], 3);
        EXPECT_EQ(compressed_data[2], 2);
        EXPECT_EQ(compressed_data[3], 1);
    }

    TEST(DRLE_range, Slope_two) {
        std::vector<std::uint32_t> data = {1, 3, 5, 7};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 4);
        EXPECT_EQ(compressed_data[0], 1);
        EXPECT_EQ(compressed_data[1], 3);
        EXPECT_EQ(compressed_data[2], 5);
        EXPECT_EQ(compressed_data[3], 7);
    }

    TEST(DRLE_range, Slope_negative_two) {
        std::vector<std::uint32_t> data = {7, 5, 3, 1};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 4);
        EXPECT_EQ(compressed_data[0], 7);
        EXPECT_EQ(compressed_data[1], 5);
        EXPECT_EQ(compressed_data[2], 3);
        EXPECT_EQ(compressed_data[3], 1);
    }

    TEST(DRLE_range, Slope_half) {
        std::vector<std::uint32_t> data = {1, 1, 2, 2};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 4);
        EXPECT_EQ(compressed_data[0], 1);
        EXPECT_EQ(compressed_data[1], 1);
        EXPECT_EQ(compressed_data[2], 2);
        EXPECT_EQ(compressed_data[3], 2);
    }

    TEST(DRLE_range, Slope_one_one) {
        std::vector<std::uint32_t> data = {1, 2, 1, 2};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 4);
        EXPECT_EQ(compressed_data[0], 1);
        EXPECT_EQ(compressed_data[1], 2);
        EXPECT_EQ(compressed_data[2], 1);
        EXPECT_EQ(compressed_data[3], 2);
    }

    TEST(DRLE_range, Slope_half_half) {
        std::vector<std::uint32_t> data = {1, 1, 2, 2, 1, 1, 2, 2};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 8);
        EXPECT_EQ(compressed_data[0], 1);
        EXPECT_EQ(compressed_data[1], 1);
        EXPECT_EQ(compressed_data[2], 2);
        EXPECT_EQ(compressed_data[3], 2);
        EXPECT_EQ(compressed_data[4], 1);
        EXPECT_EQ(compressed_data[5], 1);
        EXPECT_EQ(compressed_data[6], 2);
        EXPECT_EQ(compressed_data[7], 2);
    }

    TEST(DRLE_range, Slope_neg_one_neg_one) {
        std::vector<std::uint32_t> data = {2, 1, 2, 1};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 4);
        EXPECT_EQ(compressed_data[0], 2);
        EXPECT_EQ(compressed_data[1], 1);
        EXPECT_EQ(compressed_data[2], 2);
        EXPECT_EQ(compressed_data[3], 1);
    }

    TEST(DRLE_range, Slope_neg_two_neg_two) {
        std::vector<std::uint32_t> data = {2, 0, 2, 0};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 4);
        EXPECT_EQ(compressed_data[0], 2);
        EXPECT_EQ(compressed_data[1], 0);
        EXPECT_EQ(compressed_data[2], 2);
        EXPECT_EQ(compressed_data[3], 0);
    }

    TEST(DRLE_range, Slope_neg_half_neg_half) {
        std::vector<std::uint32_t> data = {2, 2, 1, 1, 2, 2, 1, 1};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 8);
        EXPECT_EQ(compressed_data[0], 2);
        EXPECT_EQ(compressed_data[1], 2);
        EXPECT_EQ(compressed_data[2], 1);
        EXPECT_EQ(compressed_data[3], 1);
        EXPECT_EQ(compressed_data[4], 2);
        EXPECT_EQ(compressed_data[5], 2);
        EXPECT_EQ(compressed_data[6], 1);
        EXPECT_EQ(compressed_data[7], 1);
    }
    
    TEST(DRLE_range_iterator, Dereference) {
        std::vector<std::uint32_t> data = {2, 2, 1, 1, 2, 2, 1, 1};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 8);

        auto it = compressed_data.begin();

        EXPECT_EQ(*it, 2);
    }

    TEST(DRLE_range_iterator, Preincrment_and_dereference) {
        std::vector<std::uint32_t> data = {2, 2, 1, 1, 2, 2, 1, 1};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 8);

        auto it = compressed_data.begin();

        EXPECT_EQ(*it, 2);
        ++it;
        EXPECT_EQ(*it, 2);
        ++it;
        EXPECT_EQ(*it, 1);
        ++it;
        EXPECT_EQ(*it, 1);
        ++it;
        EXPECT_EQ(*it, 2);
        ++it;
        EXPECT_EQ(*it, 2);
        ++it;
        EXPECT_EQ(*it, 1);
        ++it;
        EXPECT_EQ(*it, 1);
    }

    TEST(DRLE_range_iterator, Postincrment_and_dereference) {
        std::vector<std::uint32_t> data = {2, 2, 1, 1, 2, 2, 1, 1};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 8);

        auto it = compressed_data.begin();

        EXPECT_EQ(*(it++), 2);
        EXPECT_EQ(*(it++), 2);
        EXPECT_EQ(*(it++), 1);
        EXPECT_EQ(*(it++), 1);
        EXPECT_EQ(*(it++), 2);
        EXPECT_EQ(*(it++), 2);
        EXPECT_EQ(*(it++), 1);
        EXPECT_EQ(*(it++), 1);
    }

    TEST(DRLE_range_iterator, Predecrement_and_dereference) {
        std::vector<std::uint32_t> data = {2, 2, 1, 1, 2, 2, 1, 1};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 8);

        auto it = compressed_data.end();

        --it;
        EXPECT_EQ(*it, 1);
        --it;
        EXPECT_EQ(*it, 1);
        --it;
        EXPECT_EQ(*it, 2);
        --it;
        EXPECT_EQ(*it, 2);
        --it;
        EXPECT_EQ(*it, 1);
        --it;
        EXPECT_EQ(*it, 1);
        --it;
        EXPECT_EQ(*it, 2);
        --it;
        EXPECT_EQ(*it, 2);
    }

    TEST(DRLE_range_iterator, Postdecrement_and_dereference) {
        std::vector<std::uint32_t> data = {2, 2, 1, 1, 2, 2, 1, 1};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 8);

        auto it = compressed_data.end();

        it--;
        EXPECT_EQ(*(it--), 1);
        EXPECT_EQ(*(it--), 1);
        EXPECT_EQ(*(it--), 2);
        EXPECT_EQ(*(it--), 2);
        EXPECT_EQ(*(it--), 1);
        EXPECT_EQ(*(it--), 1);
        EXPECT_EQ(*(it--), 2);
        EXPECT_EQ(*(it),   2);
    }

    TEST(DRLE_range_iterator, Add_assign_positive_and_dereference) {
        std::vector<std::uint32_t> data = {2, 2, 1, 1, 2, 2, 1, 1};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 8);

        auto it0 = compressed_data.begin();
        it0 += 0;
        EXPECT_EQ(*it0, 2);

        auto it1 = compressed_data.begin();
        it1 += 1;
        EXPECT_EQ(*it1, 2);

        auto it2 = compressed_data.begin();
        it2 += 2;
        EXPECT_EQ(*it2, 1);

        auto it3 = compressed_data.begin();
        it3 += 3;
        EXPECT_EQ(*it3, 1);

        auto it4 = compressed_data.begin();
        it4 += 4;
        EXPECT_EQ(*it4, 2);

        auto it5 = compressed_data.begin();
        it5 += 5;
        EXPECT_EQ(*it5, 2);

        auto it6 = compressed_data.begin();
        it6 += 6;
        EXPECT_EQ(*it6, 1);

        auto it7 = compressed_data.begin();
        it7 += 7;
        EXPECT_EQ(*it7, 1);
    }

    TEST(DRLE_range_iterator, Sub_assign_positive_and_dereference) {
        std::vector<std::uint32_t> data = {2, 2, 1, 1, 2, 2, 1, 1};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 8);

        auto it0 = compressed_data.end();
        it0 -= 1;
        EXPECT_EQ(*it0, 1);

        auto it1 = compressed_data.end();
        it1 -= 2;
        EXPECT_EQ(*it1, 1);

        auto it2 = compressed_data.end();
        it2 -= 3;
        EXPECT_EQ(*it2, 2);

        auto it3 = compressed_data.end();
        it3 -= 4;
        EXPECT_EQ(*it3, 2);

        auto it4 = compressed_data.end();
        it4 -= 5;
        EXPECT_EQ(*it4, 1);

        auto it5 = compressed_data.end();
        it5 -= 6;
        EXPECT_EQ(*it5, 1);

        auto it6 = compressed_data.end();
        it6 -= 7;
        EXPECT_EQ(*it6, 2);

        auto it7 = compressed_data.end();
        it7 -= 8;
        EXPECT_EQ(*it7, 2);
    }

    TEST(DRLE_range_iterator, Add_assign_negative_and_dereference) {
        std::vector<std::uint32_t> data = {2, 2, 1, 1, 2, 2, 1, 1};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 8);

        auto it0 = compressed_data.end();
        it0 += -1;
        EXPECT_EQ(*it0, 1);

        auto it1 = compressed_data.end();
        it1 += -2;
        EXPECT_EQ(*it1, 1);

        auto it2 = compressed_data.end();
        it2 += -3;
        EXPECT_EQ(*it2, 2);

        auto it3 = compressed_data.end();
        it3 += -4;
        EXPECT_EQ(*it3, 2);

        auto it4 = compressed_data.end();
        it4 += -5;
        EXPECT_EQ(*it4, 1);

        auto it5 = compressed_data.end();
        it5 += -6;
        EXPECT_EQ(*it5, 1);

        auto it6 = compressed_data.end();
        it6 += -7;
        EXPECT_EQ(*it6, 2);

        auto it7 = compressed_data.end();
        it7 += -8;
        EXPECT_EQ(*it7, 2);
    }

    TEST(DRLE_range_iterator, Sub_assign_negative_and_dereference) {
        std::vector<std::uint32_t> data = {2, 2, 1, 1, 2, 2, 1, 1};

        aul::DRLE_range<std::uint32_t> compressed_data{data.begin(), data.end()};
        ASSERT_FALSE(compressed_data.empty());
        ASSERT_EQ(compressed_data.size(), 8);

        auto it0 = compressed_data.begin();
        it0 -= -0;
        EXPECT_EQ(*it0, 2);

        auto it1 = compressed_data.begin();
        it1 -= -1;
        EXPECT_EQ(*it1, 2);

        auto it2 = compressed_data.begin();
        it2 -= -2;
        EXPECT_EQ(*it2, 1);

        auto it3 = compressed_data.begin();
        it3 -= -3;
        EXPECT_EQ(*it3, 1);

        auto it4 = compressed_data.begin();
        it4 -= -4;
        EXPECT_EQ(*it4, 2);

        auto it5 = compressed_data.begin();
        it5 -= -5;
        EXPECT_EQ(*it5, 2);

        auto it6 = compressed_data.begin();
        it6 -= -6;
        EXPECT_EQ(*it6, 1);

        auto it7 = compressed_data.begin();
        it7 -= -7;
        EXPECT_EQ(*it7, 1);
    }

    //TODO: Add tests for edge cases relating to max integer values.

}

#endif //AUL_DRLE_RANGE_TESTS_HPP

#ifndef AUL_DYNAMIC_ENUM_TESTS_HPP
#define AUL_DYNAMIC_ENUM_TESTS_HPP

#include <gtest/gtest.h>

#include <aul/Dynamic_enum.hpp>

namespace aul_tests {

    TEST(Dynamic_enum, Default_value) {
        class Test_tag0;

        using enum_type = aul::Dynamic_enum<Test_tag0>;

        enum_type e0{};
        enum_type e1{};
        EXPECT_EQ(0, static_cast<int>(e0));
        EXPECT_EQ(0, static_cast<int>(e1));
        EXPECT_EQ(e0, e1);
    }

}

#endif //AUL_DYNAMIC_ENUM_TESTS_HPP

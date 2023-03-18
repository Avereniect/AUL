#ifndef AUL_DYNAMIC_ENUM_TESTS_HPP
#define AUL_DYNAMIC_ENUM_TESTS_HPP

#include <gtest/gtest.h>

#include <aul/Dynamic_enum.hpp>

namespace aul_tests {

    TEST(Dynamic_enum, Default_value) {
        class Test_tag;

        using enum_type = aul::Dynamic_enum<Test_tag>;

        enum_type e0{};
        enum_type e1{};
        EXPECT_EQ(0, static_cast<int>(e0));
        EXPECT_EQ(0, static_cast<int>(e1));
        EXPECT_EQ(e0, e1);
    }

    TEST(Dynamic_enum, Get_or_create_value) {
        class Test_tag;

        using enum_type = aul::Dynamic_enum<Test_tag>;

        enum_type e0 = enum_type::get_or_create_value("position");
        enum_type e1 = enum_type::get_or_create_value("position");

        EXPECT_EQ(e0, e1);

        enum_type e2 = enum_type::get_or_create_value("normal");

        EXPECT_NE(e0, e2);
    }

}

#endif //AUL_DYNAMIC_ENUM_TESTS_HPP

//
// Created by avereniect on 9/26/20.
//

#ifndef AUL_UTILITY_TESTS_HPP
#define AUL_UTILITY_TESTS_HPP

#include <aul/Utility.hpp>
#include <type_traits>
#include <gtest/gtest.h>

namespace aul::tests {

    namespace first_type_tests {

        using first0 = aul::first_type<int>::type;
        using first1 = aul::first_type<int, float>::type;
        using first2 = aul::first_type<int, float, long>::type;

        using first0t = aul::first_type_t<int>;
        using first1t = aul::first_type_t<int, float>;
        using first2t = aul::first_type_t<int, float, long>;

        static_assert(std::is_same_v<first0, int>);
        static_assert(std::is_same_v<first1, int>);
        static_assert(std::is_same_v<first2, int>);

        static_assert(std::is_same_v<first0t, int>);
        static_assert(std::is_same_v<first1t, int>);
        static_assert(std::is_same_v<first2t, int>);

    }

    namespace are_same_types_tests {

        constexpr bool same_types0 = aul::are_same_types<int>::value;
        constexpr bool same_types1 = aul::are_same_types<int, int>::value;
        constexpr bool same_types2 = aul::are_same_types<int, int, int>::value;

        static_assert(same_types0);
        static_assert(same_types1);
        static_assert(same_types2);

        constexpr bool same_types3 = aul::are_same_types<int, float>::value;
        constexpr bool same_types4 = aul::are_same_types<int, float, double>::value;


        static_assert(!same_types3);
        static_assert(!same_types4);



        constexpr bool same_types5 = aul::are_same_types_v<int>;
        constexpr bool same_types6 = aul::are_same_types_v<int, int>;
        constexpr bool same_types7 = aul::are_same_types_v<int, int, int>;

        static_assert(same_types5);
        static_assert(same_types6);
        static_assert(same_types7);

        constexpr bool same_types8 = aul::are_same_types_v<int, float>;
        constexpr bool same_types9 = aul::are_same_types_v<int, float, double>;

        static_assert(!same_types8);
        static_assert(!same_types9);

    }

    namespace pack_into_array_tests {

        const auto arr0 = aul::pack_into_array<1>(1);
        const auto arr1 = aul::pack_into_array<2>(1, 2);
        const auto arr2 = aul::pack_into_array<3>(1, 2, 3);

        TEST(pack_into_array, contents) {
            EXPECT_EQ(arr0.size(), 1);
            EXPECT_EQ(arr1.size(), 2);
            EXPECT_EQ(arr2.size(), 3);

            EXPECT_EQ(arr0[0], 1);

            EXPECT_EQ(arr1[0], 1);
            EXPECT_EQ(arr2[1], 2);

            EXPECT_EQ(arr1[0], 1);
            EXPECT_EQ(arr2[1], 2);
            EXPECT_EQ(arr2[2], 3);
        }

    }

}

#endif //AUL_UTILITY_TESTS_HPP

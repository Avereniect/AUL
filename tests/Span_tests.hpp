//
// Created by avereniect on 6/22/22.
//

#ifndef AUL_SPAN_TESTS_HPP
#define AUL_SPAN_TESTS_HPP

#include <gtest/gtest.h>

#include "aul/Span.hpp"

namespace aul_tests {

    using namespace aul;

    TEST(Span_tests, Constructors) {
        const int length = 8;
        int data[length] {
            0, 1, 2, 3
        };

        aul::Span<int> span{data, length};
    }


}

#endif //AUL_SPAN_TESTS_HPP

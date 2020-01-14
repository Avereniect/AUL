#ifndef AUL_ASSOCIATIVE_LIST_TESTS_HPP
#define AUL_ASSOCIATIVE_LIST_TESTS_HPP


#include "../../aul/containers/Associative_array.hpp"
#include <catch2/catch.hpp>


TEST_CASE("aul::associative_list-empty", "container") {
    using aul::Associative_array;
    Associative_array<int, float> map;

    CHECK(map.empty());

    CHECK(map.size() == 0);

    bool begin_equals_end = (map.begin() == map.end());
    CHECK(begin_equals_end);

    bool cbegin_equals_begin = (map.cbegin() == map.begin());
    CHECK(cbegin_equals_begin);

    bool cend_equals_end = (map.cend() == map.end());
    CHECK(cend_equals_end);

    CHECK(map.capacity() == 0);
    CHECK(map.max_size() != 0);

    CHECK(map.data() == nullptr);
    CHECK(map.keys() == nullptr);

    bool default_equals_zero_initialized = (map == Associative_array<int, float>(0));
    CHECK(default_equals_zero_initialized);
}

#endif //AUL_ASSOCIATIVE_LIST_HPP

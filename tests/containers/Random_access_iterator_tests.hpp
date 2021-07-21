#ifndef AUL_RANDOM_ACCESS_ITERATOR_TESTS_HPP
#define AUL_RANDOM_ACCESS_ITERATOR_TESTS_HPP

#include <aul/containers/Random_access_iterator.hpp>

#include <gtest/gtest.h>

#include <type_traits>

namespace aul::tests {

    using it_type = aul::Random_access_iterator<std::allocator<int>, false>;

    static_assert(std::is_trivial_v<it_type>);
    static_assert(std::is_copy_constructible_v<it_type>);
    static_assert(std::is_copy_assignable_v<it_type>);
    static_assert(std::is_swappable_v<it_type>);
    static_assert(std::is_destructible_v<it_type>);


}

#endif
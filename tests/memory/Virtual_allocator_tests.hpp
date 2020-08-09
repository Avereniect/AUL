//
// Created by avereniect on 7/20/20.
//

#ifndef AUL_VIRTUAL_ALLOCATOR_TESTS_HPP
#define AUL_VIRTUAL_ALLOCATOR_TESTS_HPP

#include <aul/memory/Virtual_allocator.hpp>

#include <gtest/gtest.h>

#include <type_traits>

namespace aul::tests {

    using short_float_ptr = Relative_pointer<float, short>;
    using short_int_ptr = Relative_pointer<int, short>;

    static_assert(std::is_same_v<std::pointer_traits<short_float_ptr>::pointer, short_float_ptr>);
    static_assert(std::is_same_v<std::pointer_traits<short_float_ptr>::elemment_type, float>);
    static_assert(std::is_same_v<std::pointer_traits<short_float_ptr>::rebind<int>, short_int_ptr>);

   TEST(Relative_pointer, Constructors) {
       float x;
       short_float_ptr ptr0;
       short_float_ptr ptr1 = nullptr;
       short_float_ptr ptr2 = &x;
   }

   TEST(Virtual_allocator, Default_constructor) {
       Virtual_allocator<float, std::int16_t> empty_allocator{};
       Virtual_allocator<float, std::int16_t> allocator{64};
   }

}

#endif //AUL_VIRTUAL_ALLOCATOR_TESTS_HPP

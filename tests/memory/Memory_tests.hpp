#ifndef AUL_MEMORY_TESTS_HPP
#define AUL_MEMORY_TESTS_HPP

#include <aul/memory/Memory.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <cstdint>

namespace aul::tests {

    TEST(Memory, default_construct) {
        using allocator_type = std::allocator<uint32_t>;

        auto allocator = allocator_type{};
        typename allocator_type::pointer allocation = std::allocator_traits<allocator_type>::allocate(allocator, 32);

        aul::default_construct_n<typename allocator_type::pointer, int, allocator_type>(allocation, 16, allocator);
    }

}

#endif //AUL_MEMORY_TESTS_HPP

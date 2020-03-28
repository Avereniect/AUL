#ifndef AUL_LINEAR_ALLOCATOR_TESTS_HPP
#define AUL_LINEAR_ALLOCATOR_TESTS_HPP

#include "../../aul/memory/Linear_allocator.hpp"
#include "gtest/gtest.h"

#include <memory>
#include <type_traits>

namespace aul::tests {

    TEST(Linear_allocator, Type_aliases) {
        using traits = std::allocator_traits<aul::Linear_allocator<int>>;

        EXPECT_TRUE("std::is_same_v<typename traits::value_type, int>");
        EXPECT_TRUE("std::is_same_v<typename traits::pointer, int*>");
        EXPECT_TRUE("std::is_same_v<typename traits::const_pointer, const int*>");
        EXPECT_TRUE("std::is_same_v<typename traits::void_pointer, void*>");
        EXPECT_TRUE("std::is_same_v<typename traits::const_void_pointer, const void*>");
        EXPECT_TRUE("std::is_same_v<typename traits::difference_type, std::ptrdiff_t>");
        EXPECT_TRUE("std::is_same_v<typename traits::size_type, std::size_t>");
        EXPECT_TRUE("std::is_same_v<typename traits::propagate_on_container_copy_assignment, std::true_type>");
        EXPECT_TRUE("std::is_same_v<typename traits::propagate_on_container_move_assignment, std::true_type>");
        EXPECT_TRUE("std::is_same_v<typename traits::propagate_on_container_swap, std::true_type>");
        EXPECT_TRUE("std::is_same_v<typename traits::is_always_equal, std::false_type>");
    }

    TEST(Linear_allocator, Rebind) {
        using rebound_allocator = typename aul::Linear_allocator<float>:: template rebind<long>::other;
        EXPECT_TRUE("std::is_same_v<rebound_allocator, aul::Linear_allocator<long>>");
    }

    TEST(Linear_allocator, Default_constructor) {
        aul::Linear_allocator<int> allocator;

        auto size = allocator.max_size();

        EXPECT_EQ(size, 0);
        EXPECT_EQ(allocator.allocate(0), nullptr);
        EXPECT_ANY_THROW(auto* ptr = allocator.allocate(1));
    }

    TEST(Linear_allocator, Single_allocation) {
        aul::Linear_allocator<int> allocator{128};
        int* mem = allocator.allocate(128);

        EXPECT_EQ(allocator.allocate(0), nullptr);
        EXPECT_NE(mem, nullptr);
        EXPECT_ANY_THROW(auto* ptr = allocator.allocate(1));
    }

    TEST(Linear_allocator, Multiple_allocations) {
        aul::Linear_allocator<int> allocator{256};

        const std::size_t NUM_ALLOCATIONS = 8;
        int* ptrs[NUM_ALLOCATIONS];

        for (int i = 0; i < NUM_ALLOCATIONS; ++i) {
            ptrs[i] = allocator.allocate(32);
        }

        for (int i = 0; i + 1 < NUM_ALLOCATIONS; ++i) {
            EXPECT_EQ(ptrs[i] + 32, ptrs[i + 1]);
        }
    }

    TEST(Linear_allocator, Vector) {
        aul::Linear_allocator<float> allocator{8};
        std::vector<float, decltype(allocator)> vec{allocator};
        vec.reserve(4);

        EXPECT_EQ(vec.get_allocator(), allocator);
        EXPECT_EQ(vec.max_size(), allocator.max_size());
    }

    TEST(Linear_allocator, Multiple_vectors) {
        aul::Linear_allocator<float> allocator{2048*2};
        std::vector<float, decltype(allocator)> vec0{allocator};
        std::vector<float, decltype(allocator)> vec1{allocator};
        vec0.reserve(1024);
        vec1.reserve(1024);

        EXPECT_EQ(vec0.get_allocator(), allocator);
        EXPECT_EQ(vec1.get_allocator(), allocator);
        EXPECT_EQ(vec0.get_allocator(), vec1.get_allocator());

        EXPECT_EQ(vec0.max_size(), allocator.max_size());
        EXPECT_EQ(vec1.max_size(), allocator.max_size());
        EXPECT_EQ(vec0.max_size(), vec1.max_size());
    }

    TEST(Linear_allocator, Multiple_types) {
        aul::Linear_allocator<float> allocator0{1024};
        aul::Linear_allocator<int> allocator1{allocator0};
        float* allocation0 = allocator0.allocate(32);
        int* allocation1 = allocator1.allocate(32);

        float* allocation2 = allocator0.allocate(0);
        int* allocation3 = allocator1.allocate(0);

        EXPECT_NE(allocation0, nullptr);
        EXPECT_NE(allocation1, nullptr);

        EXPECT_EQ(allocator0, aul::Linear_allocator<float>{allocator1});

        EXPECT_EQ(allocation2, nullptr);
        EXPECT_EQ(allocation3, nullptr);
    }

}

#endif //AUL_LINEAR_ALLOCATOR_TESTS_HPP

#ifndef AUL_MEMORY_TESTS_HPP
#define AUL_MEMORY_TESTS_HPP

#include "../memory/Memory_tests.hpp"

#include <catch2/catch.hpp>

#include <algorithm>
#include <memory>
#include <cstdint>

namespace aul::tests {

    template<typename T, class Alloc = std::allocator<T>>
    bool uninitialized_default_construct_test(const Alloc& allocator = Alloc{}) {
        constexpr std::size_t SIZE = 128;

        std::unique_ptr<T[]> array{new T[SIZE]};

        aul::uninitialized_default_construct(array.get(), array.get() + SIZE, allocator);

        T default_object = T{};
        auto predicate = [&] (const T& x) -> bool {
            return (x == default_object);
        };

        return std::all_of(array.get(), array.get() + SIZE, predicate);
    }

    template<typename T, class Alloc = std::allocator<T>>
    bool uninitialized_default_construct_n_test(const Alloc& allocator = Alloc{}) {
        constexpr std::size_t SIZE = 128;

        std::unique_ptr<T[]> array{new T[SIZE]};

        aul::uninitialized_default_construct_n(array.get(),SIZE, allocator);

        T default_object = T{};
        auto predicate = [&] (const T& x) -> bool {
            return (x == default_object);
        };

        return std::all_of(array.get(), array.get() + SIZE, predicate);
    }

}

TEST_CASE( "aul::memory::uninitialized_default_construct", "[memory]" ) {
    CHECK(aul::tests::uninitialized_default_construct_test<char>());
    CHECK(aul::tests::uninitialized_default_construct_test<double>());
    CHECK(aul::tests::uninitialized_default_construct_test<std::string>());
}

TEST_CASE( "aul::memory::uninitialized_default_construct_n", "[memory]") {
    CHECK(aul::tests::uninitialized_default_construct_n_test<char>());
    CHECK(aul::tests::uninitialized_default_construct_n_test<double>());
    CHECK(aul::tests::uninitialized_default_construct_n_test<std::string>());
}

#endif //AUL_MEMORY_TESTS_HPP

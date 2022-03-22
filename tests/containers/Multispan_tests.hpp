//
// Created by avereniect on 3/22/22.
//

#ifndef AUL_MULTISPAN_TESTS_HPP
#define AUL_MULTISPAN_TESTS_HPP

#include <aul/containers/Multispan.hpp>

namespace aul::test {

    TEST(Span_tests, Constructors) {
        std::array<std::uint32_t, 12> array{};
        std::uint32_t raw_array[12]{};

        aul::Span<std::uint32_t> span0{};
        aul::Span<std::uint32_t> span1{span0};
        aul::Span<std::uint32_t> span2{std::move(span0)};

        aul::Span<std::uint32_t> span3{array.data(), array.size()};
        aul::Span<std::uint32_t> span4{array.begin(), array.end()};
        aul::Span<std::uint32_t> span5{array};
        aul::Span<std::uint32_t> span6{raw_array};
    }

    TEST(Span_tests, Constructors_from_const) {
        const std::array<std::uint32_t, 12> array{};
        std::array<const std::uint32_t, 12> array1{};
        const std::uint32_t raw_array[12]{};

        aul::Span<const std::uint32_t> span3{array.data(), array.size()};
        aul::Span<const std::uint32_t> span4{array.begin(), array.end()};
        aul::Span<const std::uint32_t> span5{array};
        aul::Span<const std::uint32_t> span6{raw_array};
        aul::Span<const std::uint32_t> span7{array1};
    }

}

#endif //AUL_MULTISPAN_TESTS_HPP

#ifndef AUL_ALGORITHMS_HPP
#define AUL_ALGORITHMS_HPP

#include "Bits.hpp"

#include <iterator>
#include <functional>
#include <memory>

namespace aul {

    ///
    /// \tparam F_iter
    /// \tparam T
    /// \param begin
    /// \param end
    /// \param val
    /// \return
    template<typename F_iter, typename T, typename C = std::less<T>>
    constexpr F_iter linear_search(F_iter begin, F_iter end, const T& val, const C c = {}) {
        while ((begin != end) && c(*begin, val)) {
            ++begin;
        }
        return begin;
    }

    ///
    /// \tparam F_iter Forward iterator type
    /// \tparam T      Object type to be compared
    /// \param begin   Iter
    /// \param end
    /// \param val
    /// \return True if an object comparing equal to val was found.
    template<class F_iter, class T, class C = std::equal_to<T>>
    constexpr bool linear_find(F_iter begin, F_iter end, const T& val) {
        for (;begin != end; ++begin) {
            if (C(*begin, val)) {
                return true;
            }
        }
        return false;
    }

    ///
    /// \tparam R_iter Random-access iterator
    /// \tparam T Type implicitly convertible to decltype(*R_iter{})
    /// \tparam C Comparator object type
    /// \param begin Iterator to beginning of range
    /// \param end Iterator to end of range
    /// \param val Value to search for
    /// \param c Comparator object
    /// \return Iterator to location where val is expected to be, even if it's
    ///     found at that location.
    template<class R_iter, class T, class C = std::less<T>>
    [[nodiscard]]
    constexpr R_iter binary_search(R_iter begin, R_iter end, const T& val, C c = {}) {
        using diff_type = typename std::iterator_traits<R_iter>::difference_type;

        constexpr diff_type empty_full[2] = {0, ~diff_type{0}};

        diff_type size = (end - begin);
        R_iter pivot;

        while (size) {
            diff_type half = (size >> 1);
            pivot = begin + half;
            begin = begin + ((size - half) & empty_full[c(*pivot, val)]);
            size = half;
        }

        return begin;
    }

    /*
    ///
    /// \tparam R_iter Randomc access iterator type
    /// \tparam T Object Type to compare to
    /// \tparam C Comparator type
    /// \param begin Iterator to begining of range
    /// \param end Iterator to end of range
    /// \param val Value to compare against
    /// \param c Comparator object
    template<class R_iter, class T, class C = std::less<T>>
    [[nodiscard]]
    constexpr R_iter exponential_search(R_iter begin, R_iter end, const T& val, C c = {}) {
        using diff_type = std::iterator_traits<R_iter>::difference_type;

        diff_type size = (end - begin);
        diff_type offset = 1;

        R_iter a = begin;
        R_iter b = begin;
        while (b != end) {
            if (c(val, *b)) {
                return binary_search(a, b, val, c);
            }

            a = b;
            b = b + offset;

            if ((size / 2) < offset) {
                offset = size;
            } else {
                offset *= 2;
            }
        }

        return end;
    }
    */

    ///
    /// Remove consecutive elements in the range specified by [begin, end) when
    /// c(a, b) returns false.
    ///
    /// Similar to std::unique but meant to be used with ordering relations as
    /// opposed to equality relations.
    ///
    /// \tparam R_iter Forward iterator type
    /// \tparam C Comparator type
    /// \param begin Iterator to beginning of range
    /// \param end Iterator to end of range
    /// \param c Comparator object to use
    /// \return Iterator to new end of range
    template<class R_iter, class C>
    R_iter filter_adjacent(R_iter begin, R_iter end, C c = {}) {
        auto a = begin;
        auto b = ++a;
        for (;b != end; ++b) {
            if (c(*a, *b)) {
                ++a;
                *a = std::move(*b);
            }
        }

        return ++a;
    }

    template<class...Args>
    void no_op(const Args&...) {}

}

#endif //AUL_ALGORITHMS_HPP

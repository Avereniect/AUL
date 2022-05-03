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
    constexpr bool linear_find(F_iter begin, F_iter end, const T& val, C c = {}) {
        for (;begin != end; ++begin) {
            if (c(*begin, val)) {
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

    template<class...Args>
    void no_op(const Args&...) {}

}

#endif //AUL_ALGORITHMS_HPP

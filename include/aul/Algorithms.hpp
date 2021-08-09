#ifndef AUL_ALGORITHMS_HPP
#define AUL_ALGORITHMS_HPP

#include "Bits.hpp"

#include <iterator>
#include <functional>
#include <memory>

namespace aul {

    ///
    /// \tparam It Forward iterator
    /// \param begin0
    /// \param end0
    /// \param begin1
    /// \param end1
    /// \return True if range [begin0, end0) is less than [begin1, end1)
    template<class It>
    constexpr bool less_than(It begin0, It end0, It begin1, It end1) {
        return std::lexicographical_compare(begin0, end0, begin1, end1);
    }

    ///
    /// \tparam It
    /// \param begin0
    /// \param end0
    /// \param begin1
    /// \param end1
    /// \return
    template<class It>
    constexpr bool greater_than(It begin0, It end0, It begin1, It end1) {
        using value_type = typename std::iterator_traits<It>::value_type;
        constexpr bool is_gt_comparable = requires(const value_type& t) {
            t > t;
        };

        if constexpr(is_gt_comparable) {
            return std::lexicographical_compare(begin0, end0, begin1, end1, std::greater<value_type>{});
        } else {
            return less_than(begin1, end1, begin0, end0);
        }
    }

    ///
    /// \tparam F_iter0
    /// \tparam F_iter1
    /// \param begin0
    /// \param end0
    /// \param begin1
    /// \param end1
    /// \return
    template<class It>
    constexpr bool less_than_or_equal(It begin0, It end0, It begin1, It end1) {
        using value_type = typename std::iterator_traits<It>::value_type;
        constexpr bool is_leq_comparable = requires(const value_type& t) {
            t <= t;
        };

        if constexpr (is_leq_comparable) {
            return std::lexicographical_compare(begin0, end0, begin1, end1, std::less_equal<value_type>{});
        } else {
            return !(greater_than(begin0, end0, begin1, end1));
        }
    }

    ///
    /// \tparam It
    /// \param begin0
    /// \param end0
    /// \param begin1
    /// \param end1
    /// \return
    template<class It>
    constexpr bool greater_than_or_equal(It begin0, It end0, It begin1, It end1) {
        using value_type = typename std::iterator_traits<It>::value_type;
        constexpr bool is_ge_comparable = requires(const value_type& t) {
            t >= t;
        };

        if constexpr (is_ge_comparable) {
            return std::lexicographical_compare(begin0, end0, begin1, end1, std::greater_equal<value_type>{});
        } else {
            return !(less_than(begin0, end0, begin1, end1));
        }
    }

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

    template<class...Args>
    void no_op(const Args&...) {}

}

#endif //AUL_ALGORITHMS_HPP

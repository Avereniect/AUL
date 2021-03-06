#ifndef AUL_ALGORITHMS_HPP
#define AUL_ALGORITHMS_HPP

#include "Bits.hpp"

#include <iterator>
#include <functional>
#include <memory>

namespace aul {

    ///
    /// \tparam F_iter0 Forward iterator type 0
    /// \tparam F_iter1 Forward iterator type 1
    /// \param begin0
    /// \param end0
    /// \param begin1
    /// \param end1
    /// \return True if range [begin0, end0) is less than [begin1, end1)
    template<class F_iter0, class F_iter1>
    constexpr bool less_than(F_iter0 begin0, F_iter0 end0, F_iter1 begin1, F_iter1 end1) {
        for (; (begin0 != end0) && (begin1 != end1); ++begin0, ++begin1) {
            if (*begin0 < *begin1) {
                return true;
            }
        }

        return (end0 - begin0) < (end1 - begin1);
    }

    ///
    /// \tparam F_iter0
    /// \tparam F_iter1
    /// \param begin0
    /// \param end0
    /// \param begin1
    /// \param end1
    /// \return
    template<class F_iter0, class F_iter1>
    constexpr bool less_than_or_equal(F_iter0 begin0, F_iter0 end0, F_iter1 begin1, F_iter1 end1) {
        for (; (begin0 != end0) && (begin1 != end1); ++begin0, ++begin1) {
            if (*begin0 <= *begin1) {
                return true;
            }
        }

        return (end0 - begin0) <= (end1 - begin1);
    }

    ///
    /// \tparam F_iter0
    /// \tparam F_iter1
    /// \param begin0
    /// \param end0
    /// \param begin1
    /// \param end1
    /// \return
    template<class F_iter0, class F_iter1>
    constexpr bool greater_than(F_iter0 begin0, F_iter0 end0, F_iter1 begin1, F_iter1 end1) {
        for (; (begin0 != end0) && (begin1 != end1); ++begin0, ++begin1) {
            if (*begin0 > *begin1) {
                return true;
            }
        }

        return (end0 - begin0) > (end1 - begin1);
    }

    ///
    /// \tparam F_iter0
    /// \tparam F_iter1
    /// \param begin0
    /// \param end0
    /// \param begin1
    /// \param end1
    /// \return
    template<class F_iter0, class F_iter1>
    constexpr bool greater_than_or_equal(F_iter0 begin0, F_iter0 end0, F_iter1 begin1, F_iter1 end1) {
        for (; (begin0 != end0) && (begin1 != end1); ++begin0, ++begin1) {
            if (*begin0 >= * begin1) {
                return true;
            }
        }

        return (end0 - begin0) >= (end1 - begin1);
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

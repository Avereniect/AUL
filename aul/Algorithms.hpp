#ifndef AUL_TESTS_ALGORITHMS_HPP
#define AUL_TESTS_ALGORITHMS_HPP

#include <iterator>

namespace aul {

    template<class F_iter0, class F_iter1>
    constexpr bool less_than(F_iter0 begin0, F_iter0 end0, F_iter1 begin1, F_iter1 end1) {
        for (; (begin0 != end0) && (begin1 != end1); ++begin0, ++begin1) {
            if ( ! (*begin0 < *begin1) ) {
                return false;
            }
        }

        return (end0 - begin0) < (end1 - begin1);
    }

    template<class F_iter0, class F_iter1>
    constexpr bool less_than_or_equal(F_iter0 begin0, F_iter0 end0, F_iter1 begin1, F_iter1 end1) {
        for (; (begin0 != end0) && (begin1 != end1); ++begin0, ++begin1) {
            if (!(*begin0 <= *begin1)) {
                return false;
            }
        }

        return (end0 - begin0) <= (end1 - begin1);
    }

    template<class F_iter0, class F_iter1>
    constexpr bool greater_than(F_iter0 begin0, F_iter0 end0, F_iter1 begin1, F_iter1 end1) {
        for (; (begin0 != end0) && (begin1 != end1); ++begin0, ++begin1) {
            if (!(*begin0 > *begin1)) {
                return false;
            }
        }

        return (end0 - begin0) > (end1 - begin1);
    }

    template<class F_iter0, class F_iter1>
    constexpr bool greater_than_or_equal(F_iter0 begin0, F_iter0 end0, F_iter1 begin1, F_iter1 end1) {
        for (; (begin0 != end0) && (begin1 != end1); ++begin0, ++begin1) {
            if (!(*begin0 >= * begin1)) {
                return false;
            }
        }

        return (end0 - begin0) >= (end1 - begin1);
    }

}

#endif //AUL_TESTS_ALGORITHMS_HPP

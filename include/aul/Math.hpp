#ifndef AUL_MATH_HPP
#define AUL_MATH_HPP

#include "Bits.hpp"

#include <type_traits>
#include <cmath>
#include <algorithm>
#include <vector>

namespace aul {

    //=====================================================
    // Arithmetic
    //=====================================================

    template<class T>
    [[nodiscard]]
    T divide_ceil(T x, T y) {
        static_assert(std::is_unsigned_v<T>);
        T whole = x / y;

        bool partial = (x - whole * y);

        return whole + partial;
    }

    /*
    ///
    /// \tparam T Integral type
    /// \param x numerator factor
    /// \param y numerator factor
    /// \param z denominator
    /// \return {x * y / z, x * y % z} if the result can be represented by an
    /// object of type T
    template<class T>
    [[nodiscard]]
    std::array<T, 2> mul_div(T x, T y, T z) {
        using std::max;
        using std::min;
        T a = max(x, y);
        T b = min(x, y);
        T c = z;

        T t0 = a % c;
        T t1 = a / c;

        //TODO: Complete implementation

        /*
        // o = a + b
        difference_type b = o %  ;
        difference_type a = o - b;

        // a = x * bits_per_element + y;
        difference_type x = a / bits_per_element;
        difference_type y = a % bits_per_element;

        static_assert(sizeof(std::ptrdiff_t) >= sizeof(T));

        // This could still technically overflow if std::ptrdiff_t is not at
        // least as large as bits_per_element, as unlikely as that would be.
        std::ptrdiff_t partial = size * y + size * b;
        std::ptrdiff_t whole   = size * x;

        partial += offset;
        whole += (partial / ptrdiff_t(bits_per_element));
        partial = (partial % ptrdiff_t(bits_per_element));

        ptr += whole;
        offset = partial;
        */
    }
    */
    
    template<class U, class T>
    constexpr U normalize_int(const T x) {
        static_assert(std::is_integral_v<T>);
        static_assert(std::is_floating_point_v<U>);

        constexpr U temp = std::numeric_limits<T>::max();
        return U(x) / static_cast<U>(temp);
    }

    //=====================================================
    // Utilities
    //=====================================================

    template<class T>
    T clamp(T x, T lo, T hi) {
        using std::min;
        using std::max;
        return min(max(x, lo), hi);
    }

}

#endif

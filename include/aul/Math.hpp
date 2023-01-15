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
    }
    */
    
    template<class U, class T>
    constexpr U normalize_int(const T x) {
        static_assert(std::is_integral_v<T>);
        static_assert(std::is_floating_point_v<U>);

        constexpr U temp = std::numeric_limits<T>::max();
        return U(x) / static_cast<U>(temp);
    }

}

#endif

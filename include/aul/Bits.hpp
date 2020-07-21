#ifndef AUL_BITS_HPP
#define AUL_BITS_HPP

#include <string>
#include <type_traits>
#include <limits>

namespace aul {

    template<typename T>
    [[nodiscard]]
    constexpr inline bool get_bit(const T x, const int pos) {
        static_assert(std::numeric_limits<T>::is_integer);
        return (1 << pos) & x;
    }

    template<typename T>
    [[nodiscard]]
    constexpr inline T set_bit(const T x, const int pos) {
        static_assert(std::numeric_limits<T>::is_integer);
        return x | (1 << pos);
    }

    template<typename T>
    [[nodiscard]]
    constexpr inline T clear_bit(const T x, const int pos) {
        static_assert(std::numeric_limits<T>::is_integer);
        return x & (~(1 << pos));
    }

    template<typename T>
    [[nodiscard]]
    constexpr inline T toggle_bit(const T x, const int pos) {
        static_assert(std::numeric_limits<T>::is_integer);
        return x ^ (1 << pos);
    }

    template<typename T>
    [[nodiscard]]
    constexpr inline T clear_rightmost1(const T x, const int pos) {
        static_assert(std::numeric_limits<T>::is_integer);
        using U = typename std::make_unsigned<T>::type;

        U y = static_cast<U>(x);
        return y & (y - 1);
    }

    template<typename T>
    [[nodiscard]]
    constexpr inline T set_rightmost0(const T x, const int pos) {
        static_assert(std::numeric_limits<T>::is_integer);
        using U = typename std::make_unsigned<T>::type;

        U y = static_cast<U>(x);
        return y | (y + 1);
    }

    template<typename T>
    [[nodiscard]]
    constexpr inline T clear_trailing1s(const T x, const int pos) {
        static_assert(std::numeric_limits<T>::is_integer);
        using U = typename std::make_unsigned<T>::type;

        U y = static_cast<U>(x);
        return y & (y + 1);
    }

    template<typename T>
    [[nodiscard]]
    constexpr inline T set_trailing0s(const T x, const int pos) {
        static_assert(std::numeric_limits<T>::is_integer);
        using U = typename std::make_unsigned<T>::type;

        U y = static_cast<U>(x);
        return y | (y - 1);
    }

    template<typename T>
    [[nodiscard]]
    constexpr inline T get_bit_range(const T x, const unsigned from, const unsigned to) {
        static_assert(std::numeric_limits<T>::is_integer);
        constexpr T full = ~0;
        return (x >> from) & ~(full << to);
    }

    template<typename T>
    [[nodiscard]]
    constexpr inline T set_bit_range(const T x, const unsigned from, const unsigned to) {
        static_assert(std::numeric_limits<T>::is_integer);
        constexpr T full = ~0;
        int mask = (full << from) & (full >> (std::numeric_limits<T>::digits - to) ) ;
        return x | mask;
    }

    template<typename T>
    [[nodiscard]]
    constexpr inline T clear_bit_range(const T x, const unsigned from, const unsigned to) {
        static_assert(std::numeric_limits<T>::is_integer);

        constexpr T full = ~0;
        int mask = (full << from) & (full >> (std::numeric_limits<T>::digits - to) ) ;
        return x & ~mask;
    }

    template<typename T>
    [[nodiscard]]
    constexpr inline T toggle_bit_range(const T x, const unsigned from, const unsigned to) {
        static_assert(std::numeric_limits<T>::is_integer);

        constexpr T full = ~0;
        int mask = (full << from) & (full >> (std::numeric_limits<T>::digits - to) ) ;
        return x ^ mask;
    }

    template<typename T>
    [[nodiscard]]
    constexpr inline T rotl(const T x, const int rot) {
        static_assert(std::numeric_limits<T>::is_integer);

        return (x << rot) | x >> (std::numeric_limits<T>::digits - rot);
    }

    template<typename T>
    [[nodiscard]]
    constexpr inline T rotr(const T x, const int rot) {
        static_assert(std::numeric_limits<T>::is_integer);

        return (x >> rot) | x << (std::numeric_limits<T>::digits - rot);
    }

    ///
    /// Creates a std::string representing the bit string of x
    ///
    /// \tparam T Integral type
    /// \param x  Value to convert
    /// \return   std::string conversion of x
    template<typename T>
    [[nodiscard]]
    std::string bits_to_string(const T x) {
        static_assert(std::numeric_limits<T>::is_integer);
        constexpr auto bit_width = std::numeric_limits<T>::digits;
        char array[bit_width];

        T mask = 1;
        for (int i = 0; i < std::numeric_limits<T>::digits; ++i, mask <<= 1) {
            array[bit_width - i - 1] = ( (x & mask) ? '1' : '0');
        }

        return {array};
    }

    template<class T>
    [[nodiscard]]
    constexpr bool is_pow2(const T x) {
        std::size_t count = 0;
        T y = x;
        while (y) {
            count += (y & 1);
            y >>= 1;
        }
        return count < 2;
    }

    /*
    ///
    /// Mods x by 2^n
    ///
    /// \tparam T Unsigned integral type
    /// \param x  Value to mod
    /// \param p  Power of value modding by
    /// \return   Modded value
    template<typename T, typename U>
    [[nodiscard]]
    constexpr inline T mod_power(const T x, const U p) noexcept {
        static_assert(std::is_unsigned<T>::value);

        return x & ~( (1 << p) - 1);
    }

    template<typename T, typename U>
    [[nodiscard]]
    constexpr inline T mod_power(const T x, const U p) noexcept {
        static_assert(std::is_unsigned<T>::value);
        static_assert(std::is_unsigned<U>::value);

        //2^p
        T _2ep = 1;
        for (U i = 0; i < p; ++i) {
            _2ep *= 2;
        }

        return x % _2ep;
    }
    */

    ///
    /// \tparam T
    /// \param v
    /// \return T rounded to the nearest power of two
    template<typename T>
    [[nodiscard]]
    constexpr inline T ceil2(const T v) {
        static_assert(std::is_unsigned<T>::value);
        T n = v;
        for (int i = 1; i < std::numeric_limits<T>::digits; i <<= 1) {
            n |= (n >> i);
        }

        return n;
    }

    template<typename T>
    [[nodiscard]]
    constexpr inline T floor2(const T v) {
        T n = v;
        for (int i = 1; i < std::numeric_limits<T>::digits; i <<= 1) {
            n |= (n >> i);
        }
        return n - (n >> 1);
    }

}

#endif //AUL_BITS_HPP

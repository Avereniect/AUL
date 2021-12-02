#ifndef AUL_BITS_HPP
#define AUL_BITS_HPP

#include <string>
#include <type_traits>
#include <limits>
#include <climits>

namespace aul {

    ///
    /// \tparam T Unsigned integral type
    /// \param x Value to perform left bit-wise rotation on
    /// \param s Number of bit places to rotate. Must be less than the number of
    ///     bits in x.
    /// \return Rotated value
    template<class T>
    [[nodiscard]]
    constexpr T rotl(T x, unsigned s) noexcept {
        static_assert(std::is_unsigned_v<T>);
        auto constexpr bits = std::numeric_limits<T>::digits;
        return (x << s) | (x >> (bits - s));
    }

    ///
    /// \tparam T Unsigned integral type
    /// \param x Value to perform right bit-wise rotation on
    /// \param s Number of bit places to rotate. Must be less than the number of
    ///     bits in x.
    /// \return Rotated value
    template<class T>
    [[nodiscard]]
    constexpr T rotr(T x, unsigned s) noexcept {
        static_assert(std::is_unsigned_v<T>);
        auto constexpr bits = std::numeric_limits<T>::digits;
        return (x >> s) | ((x << (bits - s)));
    }

    ///
    /// Creates a std::string representing the bit string of x
    ///
    /// \tparam T Integral type
    /// \param x  Value to convert
    /// \return   std::string conversion of x
    template<class T>
    [[nodiscard]]
    std::string bits_to_string(const T x) {
        static_assert(std::numeric_limits<T>::is_integer);
        constexpr auto bit_width = CHAR_BIT * sizeof(T);
        char array[bit_width + 1];

        T mask = 1;
        for (int i = 0; i < bit_width; ++i, mask <<= 1) {
            array[bit_width - i - 1] = ( (x & mask) ? '1' : '0');
        }

        array[bit_width] = '\0';

        return {array};
    }

    ///
    /// Mods x by 2^p
    ///
    /// \tparam T Unsigned integral type
    /// \param x  Value to mod
    /// \param p  Power of value modding by
    /// \return   Modded value
    template<class T>
    [[nodiscard]]
    constexpr inline T mod_pow2(const T x, const int p) noexcept {
        static_assert(!std::numeric_limits<T>::is_signed);

        return x & ( (1 << p) - 1);
    }

    ///
    /// \tparam T An arbitrary integral type
    /// \param v An arbitrary value
    /// \return True if v is a power of 2. False otherwise
    template<class T>
    [[nodiscard]]
    constexpr inline bool is_pow2(const T v) {
        return v && !(v & (v - 1));
    }

    template<class T>
    [[nodiscard]]
    constexpr inline unsigned pop_cnt(const T x) {
        unsigned sum = 0;
        for (; x; sum++) {
            x &= x - 1;
        }
        return sum;
    }

    template<class T>
    [[nodiscard]]
    constexpr inline T log2(T x) {
        T ret = 0;
        while (x != 0) {
            x >>= 1;
            ++ret;
        }

        return ret;
    }

    ///
    /// \tparam T An unsigned integral type
    /// \param v Value to round
    /// \return v rounded to the nearest power of two equal or greater to it
    template<class T>
    [[nodiscard]]
    constexpr inline T ceil2(T x) {
        constexpr unsigned bits = CHAR_BIT * sizeof(T);

        x--;

        for (int i = 0; i < bits; ++i) {
            x |= (x >> (1 << i));
        }

        return x + 1;
    }

    ///
    /// \tparam T An unsigned integral type
    /// \param v Value to round
    /// \return v rounded to the nearest power of two equal or less to it
    template<class T>
    [[nodiscard]]
    constexpr inline T floor2(T x) {
        constexpr unsigned bits = CHAR_BIT * sizeof(T);

        for (int i = 0; i < bits; ++i) {
            x |= (x >> (1 << i));
        }

        return x - (x >> 1);
    }

    template<class T>
    [[nodiscard]]
    constexpr inline T fill_bits(unsigned begin, unsigned end) {
        static_assert(std::is_integral_v<T>);
        constexpr auto bits_per_int = sizeof(T) * CHAR_BIT;
        T ret = (~T{0} >> (bits_per_int - (end - begin))) << begin;
        return ret;
    }

    template<class T>
    [[nodiscard]]
    constexpr inline T fill_first_n_bits(unsigned n) {
        static_assert(std::is_integral_v<T>);
        constexpr auto bits_per_int = sizeof(T) * CHAR_BIT;
        T ret = (~T{0} >> (bits_per_int - n)) ;
        return ret;
    }

}

#endif //AUL_BITS_HPP

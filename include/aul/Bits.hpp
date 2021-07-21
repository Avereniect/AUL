#ifndef AUL_BITS_HPP
#define AUL_BITS_HPP

#include <string>
#include <type_traits>
#include <limits>
#include <climits>

namespace aul {

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
    constexpr bool is_pow2(const T v) {
        return v && !(v & (v - 1));
    }

    template<class T>
    [[nodiscard]]
    constexpr inline unsigned pop_cnt(const T v) {
        unsigned sum = 0;
        for (; v; sum++) {
            v &= v - 1;
        }
        return sum;
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

    ///
    /// \tparam T An unsigned integral type
    /// \param x An arbitrary integral value
    /// \param r Number of positions to rotate. Must be less than number of bits
    ///     in T. Undefined behavior otherwise
    /// \return x with its bits rotated left
    template<class T>
    [[nodiscard]]
    constexpr inline T rotl(const T x, const unsigned r) {
        static_assert(!std::numeric_limits<T>::is_signed);

        constexpr std::size_t bit_count = sizeof(T) * CHAR_BIT;
        const int rot = mod_pow2(r, bit_count);

        return (x << rot) | x >> (bit_count - rot);
    }

    ///
    /// \tparam T An unsigned integral type
    /// \param x An arbitrary integral value
    /// \param r Number of positions to rotate. Must be less than number of bits
    ///     in T. Undefined behavior otherwise
    /// \return x with its bits rotated right
    template<class T>
    [[nodiscard]]
    constexpr inline T rotr(const T x, const unsigned r) {
        static_assert(!std::numeric_limits<T>::is_signed);

        constexpr auto bit_count = sizeof(T) * CHAR_BIT;
        const int rot = mod_pow2(r, bit_count);

        return (x >> rot) | x << (bit_count - rot);
    }

}

#endif //AUL_BITS_HPP

//
// Created by avereniect on 8/30/19.
//

#ifndef AUL_TESTS_BITS_HPP
#define AUL_TESTS_BITS_HPP

#include <limits>
#include <string>

namespace aul {

    template<typename T>
    constexpr inline bool get_bit(const T x, const int pos) {
        static_assert(std::numeric_limits<T>::is_integer);
        return (1 << pos) & x;
    }

    template<typename T>
    constexpr inline T set_bit(const T x, const int pos) {
        static_assert(std::numeric_limits<T>::is_integer);
        return x | (1 << pos);
    }

    template<typename T>
    constexpr inline T clear_bit(const T x, const int pos) {
        static_assert(std::numeric_limits<T>::is_integer);
        return x & (~(1 << pos));
    }

    template<typename T>
    constexpr inline T toggle_bit(const T x, const int pos) {
        static_assert(std::numeric_limits<T>::is_integer);
        return x ^ (1 << pos);
    }

    template<typename T>
    constexpr inline T clear_rightmost1(const T x, const int pos) {
        static_assert(std::numeric_limits<T>::is_integer);
        using U = typename std::make_unsigned<T>::type;

        U y = static_cast<U>(x);
        return y & (y - 1);
    }

    template<typename T>
    constexpr inline T set_rightmost0(const T x, const int pos) {
        static_assert(std::numeric_limits<T>::is_integer);
        using U = typename std::make_unsigned<T>::type;

        U y = static_cast<U>(x);
        return y | (y + 1);
    }

    template<typename T>
    constexpr inline T clear_trailing1s(const T x, const int pos) {
        static_assert(std::numeric_limits<T>::is_integer);
        using U = typename std::make_unsigned<T>::type;

        U y = static_cast<U>(x);
        return y & (y + 1);
    }

    template<typename T>
    constexpr inline T set_trailing0s(const T x, const int pos) {
        static_assert(std::numeric_limits<T>::is_integer);
        using U = typename std::make_unsigned<T>::type;

        U y = static_cast<U>(x);
        return y | (y - 1);
    }

    

    template<typename T>
    constexpr inline T get_bit_range(const T x, const unsigned from, const unsigned to) {
        static_assert(std::numeric_limits<T>::is_integer);
        constexpr T full = ~0;
        return (x >> from) & ~(full << to);
    }

    template<typename T>
    constexpr inline T set_bit_range(const T x, const unsigned from, const unsigned to) {
        static_assert(std::numeric_limits<T>::is_integer);
        constexpr T full = ~0;
        int mask = (full << from) & (full >> (std::numeric_limits<T>::digits - to) ) ;
        return x | mask;
    }

    template<typename T>
    constexpr inline T clear_bit_range(const T x, const unsigned from, const unsigned to) {
        static_assert(std::numeric_limits<T>::is_integer);
        constexpr T full = ~0;
        int mask = (full << from) & (full >> (std::numeric_limits<T>::digits - to) ) ;
        return x & ~mask;
    }

    template<typename T>
    constexpr inline T toggle_bit_range(const T x, const unsigned from, const unsigned to) {
        static_assert(std::numeric_limits<T>::is_integer);
        constexpr T full = ~0;
        int mask = (full << from) & (full >> (std::numeric_limits<T>::digits - to) ) ;
        return x ^ mask;
    }

    template<typename T>
    constexpr inline T rotate_bits_left(const T x, const int rot) {
        static_assert(std::numeric_limits<T>::is_integer);

        return (x << rot) | x >> (std::numeric_limits<T>::digits - rot);
    }

    template<typename T>
    constexpr inline T rotate_bits_right(const T x, const int rot) {
        static_assert(std::numeric_limits<T>::is_integer);

        return (x >> rot) | x << (std::numeric_limits<T>::digits - rot);
    }

    template<typename T>
    std::string bits_to_string(const T x) {
        static_assert(std::numeric_limits<T>::is_integer);
        std::string str;
        str.resize(std::numeric_limits<T>::digits);

        T mask = 1;
        for (int i = 0; i < std::numeric_limits<T>::digits; ++i, mask <<= 1) {
            str[str.size() - i - 1] = ( (x & mask) ? '1' : '0');
        }

        return str;
    }

}

#endif //AUL_TESTS_BITS_HPP

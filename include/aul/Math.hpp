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
    T divide_ceil(T x, T y) {
        static_assert(std::is_unsigned_v<T>);
        T whole = x / y;

        bool partial = (x - whole * y);

        return whole + partial;
    }
    
    template<class U, class T>
    constexpr U normalize_int(const T x) {
        static_assert(std::is_integral_v<T>);
        static_assert(std::is_floating_point_v<U>);

        constexpr U temp = std::numeric_limits<T>::max();
        return U(x) / static_cast<U>(temp);
    }

    //=====================================================
    // Simple hash
    //=====================================================

    ///
    /// http://burtleburtle.net/bob/c/lookup3.c
    /// https://github.com/imageworks/OpenShadingLanguage/blob/ffc5303dcfd63cf395d3a1b6fbf6ca3894b44d5e/src/include/OSL/oslnoise.h
    ///
    template<class T>
    constexpr std::uint32_t byte_hash32(const T* data, const std::size_t n) {
        auto mix = [] (uint32_t& a, uint32_t& b, uint32_t& c) {
            a -= c; a ^= aul::rotl(c, 4);  c += b;
            b -= a;  b ^= aul::rotl(a, 6);  a += c;
            c -= b;  c ^= aul::rotl(b, 8);  b += a;
            a -= c;  a ^= aul::rotl(c,16);  c += b;
            b -= a;  b ^= aul::rotl(a,19);  a += c;
            c -= b;  c ^= aul::rotl(b, 4);  b += a;
        };

        auto final = [] (const uint32_t& x, const uint32_t& y, const uint32_t& z) -> int32_t {
            uint32_t a = x;
            uint32_t b = y;
            uint32_t c = z;

            c ^= b; c -= aul::rotl(b,14);
            a ^= c; a -= aul::rotl(c,11);
            b ^= a; b -= aul::rotl(a,25);
            c ^= b; c -= aul::rotl(b,16);
            a ^= c; a -= aul::rotl(c,4);
            b ^= a; b -= aul::rotl(a,14);
            c ^= b; c -= aul::rotl(b,24);
            return c;
        };

        const char* ptr = reinterpret_cast<const char*>(data);
        uint32_t a = 0xdeadbeef + n + 13;
        uint32_t b = a;
        uint32_t c = a;
        uint32_t length = n;

        while(length > 3) {
            a += ptr[0];
            b += ptr[1];
            c += ptr[2];

            mix(a, b, c);
            length -= 3;
            ptr += 3;
        }

        switch (length) {
            case 3:
                c += ptr[2];
            case 2:
                b += ptr[1];
            case 1:
                a += ptr[0];
                c = final(a, b, c);
            default:
                ; //Do nothing
        }

        return c;
    }

    template<class T>
    constexpr std::uint32_t byte_hash32(const T& data) {
        return byte_hash32(std::addressof(data), sizeof(data));
    }

    //=====================================================
    // Utilities
    //=====================================================

    template<class T>
    T clamp(T x, T lo, T hi) {
        return std::min(std::max(x, lo), hi);
    }

}

#endif

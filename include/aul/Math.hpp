#ifndef AUL_MATH_HPP
#define AUL_MATH_HPP

#include "Bits.hpp"

#include <type_traits>
#include <cmath>
#include <algorithm>
#include <vector>

namespace aul {

    //=====================================================
    // Interpolation functions
    //=====================================================

    template<typename T, typename U>
    U smooth_step(T fac, U a, U b) {
        static_assert(std::is_floating_point<T>::value, "T must be a floating point type");

        U x = std::clamp((fac - a) / (fac - b), 0.0, 1.0);

        return x * x * (3.0 - (2.0 * x));
    }

    template<typename T>
    T smoother_step(T y) {
        static_assert(std::is_floating_point<T>::value, "T must be a floating point type");

        T x = std::clamp(y, 0.0, 1.0);
        return x * x * x * (x * (x * 6 - 15) + 10);
    }

    //=====================================================
    // Distance metrics
    // TODO: Specializations for standard types supported by SIMD ops
    //=====================================================

    template<typename T>
    constexpr T euclidean_distance(const T p0[], const T p1[], const std::size_t n) {
        static_assert(std::is_floating_point<T>::value, "T must be a floating point type");

        T sum = 0.0;
        for (std::size_t i = 0; i < n; ++i) {
            sum += (p0[i] - p1[i]) * (p0[i] - p1[i]);
        }

        return std::sqrt(sum);
    }

    template<typename T>
    constexpr T chebyshev_distance(const T p0[], const T p1[], const std::size_t n) {
        T dist = 0.0;
        for (std::size_t i = 0; i < n; ++i) {
            dist = std::max(dist, std::abs(p0[i] - p1[i]));
        }

        return dist;
    }

    template<typename T>
    constexpr T manhattan_distance(const T p0[], const T p1[], const std::size_t n) {

        T dist = 0.0;
        for (std::size_t i = 0; i < n; ++i) {
            dist += std::abs(p0[i] - p1[i]);
        }

        return dist;
    }

    template<typename T>
    constexpr T minkowski_distance(const T p0[], const T p1[], const std::size_t n, const T p = 1.0) {
        static_assert(std::is_floating_point<T>::value, "T must be a floating point type");

        T dist = 0.0;
        for (std::size_t i = 0; i < n; ++i) {
            dist += std::pow(std::abs(p0[i] - p1[i]));
        }

        return std::pow(dist, 1.0 / p);
    }
    
    template<class U, class T>
    constexpr U normalize_int(const T x) {
        static_assert(std::is_integral_v<T>);
        static_assert(std::is_floating_point_v<U>);

        constexpr U temp = std::numeric_limits<T>::max();
        return U(x) / static_cast<U>(temp);
    }

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

}

#endif

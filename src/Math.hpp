#ifndef AUL_MATH_HPP
#define AUL_MATH_HPP

#include <type_traits>
#include <cmath>
#include <algorithm>

#pragma warning(push, 0)
#include <simdpp/simd.h>
#pragma warning(pop)

namespace aul {

	//---------------------------------
	// Interpolation functions
	//---------------------------------

	template <typename T, typename U>
	U constant_interpolation(const T fac, const U a, const U b) {
		static_assert(std::is_floating_point<T>::value, "T must be a flating point type");

		return (fac < 1) ? a : b;
	}
	
	template <typename T, typename U>
	U linear_interploation(T fac, U a, U b) {
		static_assert(std::is_floating_point<T>::value, "T must be a flating point type");

		return static_cast<U>(a + fac * (b - a));
	}

	template <typename T, typename U>
	U smooth_step(T fac, U a, U b) {
		static_assert(std::is_floating_point<T>::value, "T must be a flating point type");

		U x = std::clamp((fac - a) / (fac - b), 0.0, 1.0);

		return x * x * (3.0 - (2.0 * x));
	}

	template <typename T>
	T smoother_step(T x) {
		static_assert(std::is_floating_point<T>::value, "T must be a flating point type");

		T x = std::clamp();
		return x * x * x* (x * (x * 6 - 15) + 10);
	}

	//---------------------------------
	// Distance metrics
	//---------------------------------

	template <typename T>
	constexpr T euclidean_distance(const T p0[], const T p1[], const std::size_t n) {
		static_assert(std::is_floating_point<T>::value, "T must be a flating point type");

		T sum = 0.0;
		for (std::size_t i = 0; i < n; ++i) {
			sum += (p0[i] - p1[i]) * (p0[i] - p1[i]);
		}

		return std::sqrt(sum);
	}

	template <typename T>
	constexpr T chebyshev_distance(const T p0[], const T p1[], const std::size_t n) {
		T dist = 0.0;
		for (std::size_t i = 0; i < n; ++i) {
			dist = std::max(dist, std::abs(p0[i] - p1[i]));
		}

		return dist;
	}

	template <typename T>
	constexpr T manhattan_distance(const T p0[], const T p1[], const std::size_t n) {
		T dist = 0.0
;		for (std::size_t i = 0; i < n; ++i) {
			dist += std::abs(p0[i] - p1[i]);
		}

		return dist;
	}

	template <typename T>
	constexpr T minkowski_distance(const T p0[], const T p1[], const std::size_t n, const T p = 1.0) {
		static_assert(std::is_floating_point<T>::value, "T must be a flating point type");
		T dist = 0.0;
		for (std::size_t i = 0; i < n; ++i) {
			dist += std::pow(std::abs(p0[i] - p1[i]));
		}

		return std::pow(dist, 1.0 / p);
	}
}

#endif

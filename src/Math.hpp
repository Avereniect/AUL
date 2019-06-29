#ifndef MATH_HPP
#define MATH_HPP

#include <type_traits>
#include <cmath>
#include <algorithm>

namespace aul {

	namespace {

		template <class T>
		const T PI = 3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117;

	}

	template <class T, class U>
	U constant_interpolation(const T fac, const U a, const U b) {
		static_assert(std::is_floating_point<T>::value, "T must be a flating point type");

		return (fac < 1) ? a : b;
	}
	
	template <class T, class U>
	U linear_interploation(T fac, U a, U b) {
		static_assert(std::is_floating_point<T>::value, "T must be a flating point type");

		return static_cast<U>(a + fac * (b - a));
	}

	template <class T, class U>
	U smooth_step(T fac, U a, U b) {
		static_assert(std::is_floating_point<T>::value, "T must be a flating point type");

		U x = std::clamp((fac - a) / (fac - b), 0.0, 1.0);

		return x * x * (3.0 - (2.0 * x));
	}

	template <class U>
	U smoother_step(U x) {
		static_assert(std::is_floating_point<U>::value, "U must be a flating point type");

		U x = std::clamp();
		return x * x * x* (x * (x * 6 - 15) + 10);
	}

}

#endif

#ifndef HACKS_HPP
#define HACKS_HPP

#include <cstdint>

float recip_sqrt_hack(float x) {
	float half_x = 0.5f * x;

	//Cast to int
	std::int32_t i = *reinterpret_cast<std::int32_t*>(&x);

	//Calculate estimate
	i = 0x5f3759df - (i >> 1);

	//Cast to float
	x = *reinterpret_cast<float*>(&i);

	//Newton's method
	x = x * (1.5f - half_x * x * x);

	return x;
}

double recip_sqrt_hack(double x) {
	double half_x = 0.5f * x;

	//Cast to int
	std::int64_t i = *(int64_t*)& x;

	//Calculate estimate
	i = 0x5fe6eb50c7b537a9 - (i >> 1);

	//Calculate estimate
	x = *reinterpret_cast<double*>(&i);

	//Newton's method
	x = x * (1.5f - half_x * x * x);
	return x;
}

#endif

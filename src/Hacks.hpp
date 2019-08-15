#ifndef AUL_HACKS_HPP
#define AUL_HACKS_HPP

#include <cstdint>
#include <limits>

constexpr float recip_sqrt_hack(const float x) {
    static_assert(std::numeric_limits<float>::is_iec559);

    float half_x = 0.5f * x;

    //Cast to int
    std::int32_t i = *reinterpret_cast<std::int32_t *>(&x);

    //Calculate estimate
    i = 0x5f3759df - (i >> 1);

    //Cast to float
    x = *reinterpret_cast<float *>(&i);

    //Newton's method
    x = x * (1.5f - half_x * x * x);

    return x;
}

constexpr double recip_sqrt_hack(const double x) {
    static_assert(std::numeric_limits<double>::is_iec559);
    double half_x = 0.5f * x;

    //Cast to int
    std::int64_t i = *(int64_t *) &x;

    //Calculate estimate
    i = 0x5fe6eb50c7b537a9 - (i >> 1);

    //Cast to double
    x = *reinterpret_cast<double *>(&i);

    //Newton's method
    x = x * (1.5f - half_x * x * x);
    return x;
}

#endif

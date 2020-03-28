#ifndef AUL_BENCHMARKS_ALGORITHMS_HPP
#define AUL_BENCHMARKS_ALGORITHMS_HPP

#include "../aul/Algorithms.hpp"

//#include <celero/Celero.h>

#include <random>
#include <array>
#include <cstdint>

namespace aul::benchmarks {

    class LinearSearchFixture : public celero::TestFixture {
    public:

        LinearSearchFixture() = default;
        ~LinearSearchFixture() = default;

        virtual void setUp(const celero::TestFixture::ExperimentValue& experimentalValue) {
            auto generator = std::bind(std::uniform_int_distribution<std::uint32_t>{0, 100}, std::default_random_engine{});

            for (auto it = data.begin(); it != data.end(); ++it) {
                *it = it[-1] + generator();
            }
        }

        std::array<std::uint32_t, 1024 * 1024> data;

    };

}

BASELINE_F(Linearsearch, uint32_t, aul::benchmarks::LinearSearchFixture, 32, 16) {
    celero::DoNotOptimizeAway(aul::linear_search(data.begin(), data.end(), 9));
}

#endif

//#include "./BEN_algorithms.hpp"

//#include <celero/Celero.h>

//CELERO_MAIN

#include "../aul/Bits.hpp"
#include "../aul/Algorithms.hpp"

#include <vector>
#include <cstdlib>
#include <chrono>
#include <ios>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <typeinfo>

using clock_type = std::chrono::high_resolution_clock;
using count_t = decltype((clock_type::now() - clock_type::now()).count());

constexpr int ITERATIONS = 1024;

template<class T>
std::vector<count_t> benchmark_linear() {
    std::vector<count_t> results;
    results.reserve(ITERATIONS);

    std::vector<T> storage;
    storage.reserve(ITERATIONS);
    storage.emplace_back();
    for (int i = 1; i <= ITERATIONS; ++i) {
        auto middle_index = storage.size() - (storage.size()) / 2;

        auto begin = clock_type::now();
        volatile auto iter = aul::linear_search(storage.begin(), storage.end(), storage[middle_index]);
        auto end = clock_type::now();

        results.push_back((end - begin).count());
        storage.emplace_back(storage.back() + 1);
    }

    return results;
}

template<class T>
std::vector<count_t> benchmark_binary() {
    std::vector<count_t> results;
    results.reserve(ITERATIONS);

    return results;
}

template<class T>
void print_to_file(const std::vector<T> vec, const std::string name) {
    std::ofstream fout{name + "_results.txt"};

    for (auto x : vec) {
        fout << x << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::cout.sync_with_stdio(false);
    std::cout.tie();

    print_to_file(benchmark_linear<std::uint8_t>(), "uint8_t");
    print_to_file(benchmark_linear<std::uint16_t>(), "uint16_t");
    print_to_file(benchmark_linear<std::uint32_t>(), "uint32_t");
    print_to_file(benchmark_linear<std::uint64_t>(), "uint64_t");

    print_to_file(benchmark_linear<float>(), "float");
    print_to_file(benchmark_linear<double>(), "double");

    return EXIT_SUCCESS;
}

#ifndef AUL_TESTS_VECTOR_TESTS_HPP
#define AUL_TESTS_VECTOR_TESTS_HPP

#include "../../aul/containers/Vector.hpp"
#include <string>
#include <memory>

namespace aul::test {

    void constructor() {
        using std::string;

        std::allocator<string> alloc0;

        Vector<string> vec0_0{};
        Vector<string> vec1_0{2.0};
        Vector<string> vec2_0{alloc0};
        Vector<string> vec3_0{"Hello, World"};
        Vector<string> vec4_0{};
    }

}



#endif //AUL_TESTS_VECTOR_TESTS_HPP

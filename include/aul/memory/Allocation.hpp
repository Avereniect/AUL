#ifndef AUL_ALLOCATION_HPP
#define AUL_ALLOCATION_HPP

#include <utility>

namespace aul {

    template<class S, class P>
    struct Allocation {

        //=================================================
        // -ctors
        //=================================================

        Allocation(S capacity, P ptr):
            capacity(capacity),
            ptr(ptr) {}

        Allocation() = default;
        Allocation(const Allocation&) = delete;

        Allocation(Allocation&& a):
            capacity(std::exchange(a.capacity, 0)),
            ptr(std::exchange(a.ptr, nullptr)) {}

        ~Allocation() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Allocation& operator=(const Allocation&) = delete;

        Allocation& operator=(Allocation&& a) {
            capacity = std::exchange(a.capacity, 0);
            ptr = std::exchange(a.ptr, nullptr);
            return *this;
        }

        //=================================================
        // Instance members
        //=================================================

        size_type capacity = 0;
        pointer ptr = nullptr;

    };

}

#endif //AUL_ALLOCATION_HPP

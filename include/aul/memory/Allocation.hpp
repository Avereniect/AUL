#ifndef AUL_ALLOCATION_HPP
#define AUL_ALLOCATION_HPP

#include <memory>

namespace aul {

    template<class T, class A>
    class Allocation {
    public:

        using pointer = std::allocator_traits<A>::pointer;

        using size_type = std::allocator_traits<A>::size_type;

        //=================================================
        // Constructors
        //=================================================

        Allocation() = default;
        Allocation(const Allocation&) = default;
        Allocation(Allocation&&) noexcept = default;
        ~Allocation() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Allocation& operator=(const Allocation&) = default;
        Allocation& operator=(Allocation&&) noexcept = default;

        //=================================================
        // Instance members
        //=================================================

        pointer ptr = nullptr;
        size_type capacity = 0;

    };

}

#endif //AUL_ALLOCATION_HPP

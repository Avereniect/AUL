//
// Created by avereniect on 8/19/20.
//

#ifndef AUL_OBJECT_POOL_HPP
#define AUL_OBJECT_POOL_HPP

#include <memory>
#include <type_traits>

namespace aul {

    template<class T, class A = std::allocator<T>>
    class Pool_allocator {
    public:

        //=================================================
        // Type aliases
        //=================================================

        using value_type = T;

        template<typename>
        using pointer = typename A::pointer;

        template<typename>
        using const_pointer = typename A::const_pointer;

        template<typename>
        using void_pointer = typename A::void_pointer;

        template<typename>
        using void_pointer = typename A::void_pointer;

        template<typename>
        using const_void_pointer = typename A::const_void_pointer;

        template<typename>
        using const_void_pointer = typename A::const_void_pointer;

        template<typename>
        using size_type = typename A::size_type;

        template<typename>
        using difference_type = typename A::difference_type;

        template<class U>
        struct rebind {
            using other = Pool_allocator<U, A>;
        };

        //=================================================
        // Meta constants
        //=================================================

        using propagate_on_container_copy_assignment = std::true_type;
        using propagate_on_container_move_assignment = std::true_type;
        using propagate_on_container_swap = std::true_type;

        using is_always_equal = std::false_type;

        //=================================================
        // -ctors
        //=================================================

        Pool_allocator();
        Pool_allocator(const A&);
        Pool_allocator(A&&);
        ~Pool_allocator();

        //=================================================
        // Assignment operators
        //=================================================

        Pool_allocator& operator=(const Pool_allocator&);
        Pool_allocator& operator=(Pool_allocator&&);

        //=================================================
        // Allocation methods
        //=================================================

        //=================================================
        // Construction & destruction methods
        //=================================================

        //=================================================
        // Misc. methods
        //=================================================

    private:
        A allocator;
    };

}

#endif //AUL_OBJECT_POOL_HPP

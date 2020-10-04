//
// Created by avereniect on 9/14/20.
//

#ifndef AUL_STACK_STRATEGY_HPP
#define AUL_STACK_STRATEGY_HPP

#include <type_traits>
#include <memory>

namespace aul {

    template<class Resource, class Pointer>
    class Stack_strategy {
    public:

        using pointer = typename std::pointer_traits<Pointer>::template rebind<std::byte>;
        using difference_type = typename std::pointer_traits<Pointer>::difference_type;

        //=================================================
        // -ctors
        //=================================================

        Stack_strategy() = default;

        Stack_strategy(Resource&& resource) noexcept:
            resource(resource) {}

        ~Stack_strategy() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Stack_strategy& operator=(Stack_strategy&&) noexcept = default;

        //=================================================
        // Allocation methods
        //=================================================

        pointer allocate(const difference_type size, const difference_type alignment) {

        }

        void deallocate() {

        }

    private:
        Resource resource;
    };

}

#endif //AUL_STACK_STRATEGY_HPP

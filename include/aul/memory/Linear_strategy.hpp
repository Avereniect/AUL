//
// Created by avereniect on 8/28/20.
//

#ifndef AUL_LINEAR_STRATEGY_HPP
#define AUL_LINEAR_STRATEGY_HPP

#include <memory>

namespace aul {

    template<class Resource, class Pointer>
    class Linear_strategy {
    public:

        using pointer = typename std::pointer_traits<Pointer>::template rebind<std::byte>;
        using difference_type = typename std::pointer_traits<Pointer>::difference_type;

        //=================================================
        // -ctors
        //=================================================

        Linear_strategy(Resource&& resource):
            resource(resource) {

            resource.increment_users();

            //Initialize memory resource

        }

        ~Linear_strategy() = default;

        //=================================================
        // Allocation methods
        //=================================================

        pointer allocate(const difference_type size, const difference_type alignment) {

        }

        void deallocate() {
            //Do nothing
        }

    private:
        Resource resource{};
    };

}

#endif //AUL_LINEAR_STRATEGY_HPP

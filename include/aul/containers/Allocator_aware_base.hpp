#ifndef AUL_ALLOCATOR_AWARE_BASE_HPP
#define AUL_ALLOCATOR_AWARE_BASE_HPP

#include <memory>

namespace aul {

    template<class A>
    struct Stateful_allocator_container_base {
    protected:

        //=================================================
        // -ctors
        //=================================================

        explicit Stateful_allocator_container_base(const A& a):
            allocator(a) {}

        Stateful_allocator_container_base() = default;
        ~Stateful_allocator_container_base() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Stateful_allocator_container_base& operator=(const Stateful_allocator_container_base& rhs) {
            if constexpr (std::allocator_traits<A>::propagate_on_container_copy_assignment::value) {
                allocator = rhs.allocator;
            }

            return *this;
        }

        Stateful_allocator_container_base& operator=(Stateful_allocator_container_base&& rhs) {
            if constexpr (std::allocator_traits<A>::propagate_on_container_move_assignment::value) {
                allocator = std::move(rhs.allocator);
            }

            return *this;
        }

        //=================================================
        // Assignment operators
        //=================================================

        bool operator==(Stateful_allocator_container_base& rhs) {
            return (allocator == rhs.allocator);
        }

        bool operator!=(Stateful_allocator_container_base& rhs) {
            return (allocator != rhs.allocator);
        }

        //=================================================
        // Accessors
        //=================================================

        A get_allocator() const {
            return allocator;
        }

        //=================================================
        // Misc. methods
        //=================================================

        void swap(Stateful_allocator_container_base& rhs) {
            std::swap(allocator, rhs.allocator);
        }

        //=================================================
        // Instance members
        //=================================================

        A allocator;

    };

    template<class A>
    struct Empty_allocator_container_base {
    protected:

        //=================================================
        // -ctors
        //=================================================

        explicit Empty_allocator_container_base(const A& a) {};

        Empty_allocator_container_base() = default;
        ~Empty_allocator_container_base() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Empty_allocator_container_base& operator=(const Empty_allocator_container_base& rhs) = default;
        Empty_allocator_container_base& operator=(const Empty_allocator_container_base&& rhs) noexcept = default;

        //=================================================
        // Assignment operators
        //=================================================

        bool operator==(Empty_allocator_container_base& rhs) {
            return true;
        }

        bool operator!=(Empty_allocator_container_base& rhs) {
            return false;
        }

        //=================================================
        // Accessors
        //=================================================

        A get_allocator() const {
            return A{};
        }

        //=================================================
        // Misc. methods
        //=================================================

        void swap(Empty_allocator_container_base& rhs) {}

    };

    template<class A>
    struct Allocator_aware_base : public std::conditional_t<
        std::allocator_traits<A>::is_always_equal::value,
        Empty_allocator_container_base<A>,
        Stateful_allocator_container_base<A>
    > {

        using base = std::conditional_t<
            std::allocator_traits<A>::is_always_equal::value,
            Empty_allocator_container_base<A>,
            Stateful_allocator_container_base<A>
        >;

        using base::base;

    };

}

#endif //AUL_ALLOCATOR_AWARE_BASE_HPP

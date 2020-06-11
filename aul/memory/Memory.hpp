#ifndef AUL_MEMORY_HPP
#define AUL_MEMORY_HPP

#include <memory>
#include <vector>
#include <type_traits>

namespace aul {

    ///
    /// Allocator extended version of std::destroy
    ///
    template<class Forward_iter, class Alloc>
    void destroy(Forward_iter first, Forward_iter last, Alloc& alloc) {
        for (; first != last; ++first) {
            std::allocator_traits<Alloc>::destroy(alloc, std::addressof(*first));
        }
    }

    ///
    /// Allocator extended version of std::destroy_n
    ///
    template<class Forward_iter, class size_type, class Alloc>
    void destroy_n(Forward_iter first, size_type n, Alloc& alloc) {
        for (size_type i = size_type{}; i != n; ++i, ++first) {
            std::allocator_traits<Alloc>::destroy(alloc, std::addressof(*first));
        }
    }

    ///
    /// Allocator extended version of std::uninitialized_default_construct
    ///
    template<class Forward_iter, class Alloc>
    void default_construct(Forward_iter begin, Forward_iter end, Alloc& alloc) {
        Forward_iter x = begin;
        try {
            for (; x != end; ++x) {
                std::allocator_traits<Alloc>::construct(alloc, std::addressof(*x));
            }

        } catch (...) {
            aul::destroy(begin, x, alloc);

            throw;
        }
    }

    ///
    /// Element count version of aul::uninitialized_default_construct
    ///
    template<class Forward_iter, class size_type, class Alloc>
    void default_construct_n(Forward_iter begin, size_type n, Alloc& alloc) {
        Forward_iter x = begin;
        size_type i = size_type{};

        try {
            for (; i != n; ++i, ++x) {

                std::allocator_traits<Alloc>::construct(alloc, std::addressof(*x));
            }
        } catch (...) {
            aul::destroy_n(begin, i, alloc);

            throw;
        }
    }

    ///
    /// Allocator extended version of std::uninitialized_fill
    ///
    template<class Forward_iter, class T, class Alloc>
    void uninitialized_fill(Forward_iter begin, Forward_iter end, const T& value, Alloc allocator) {
        Forward_iter x = begin;

        try {
            for (; x != end; ++x) {
                std::allocator_traits<Alloc>::construct(allocator, std::addressof(*x), value);
            }

        } catch (...) {
            aul::destroy(begin, x, allocator);

            throw;
        }
    }

    template<class Forward_iter, class size_type, class T, class Alloc>
    void uninitialized_fill_n(Forward_iter begin, const size_type n, const T& value, Alloc& alloc) {
        Forward_iter x = begin;
        size_type i = size_type{};

        try {
            for (; i != n; ++i, ++x) {
                std::allocator_traits<Alloc>::construct(alloc, std::addressof(*x), value);
            }
        } catch (...) {
            aul::destroy_n(begin, i, alloc);

            throw;
        }
    }

    template<class Input_iter, class Forward_iter, class Alloc>
    void uninitialized_move(Input_iter begin, Input_iter end, Forward_iter dest, Alloc& allocator) {
        Forward_iter x = dest;
        Input_iter i = begin;
        try {
            for (; i != end; ++i, ++x) {
                std::allocator_traits<Alloc>::construct(allocator, std::addressof(*x), std::move(*i));
            }

        } catch (...) {
            --x;
            for (; i-- > begin; --x) {
                *i = std::move(*x);
                std::allocator_traits<Alloc>::destroy(allocator, std::addressof(*x));
            }

            throw;
        }
    }

    template<class Input_iter, class Forward_iter, class size_type, class Alloc>
    void uninitialized_move_n(Input_iter begin, size_type n, Forward_iter dest, Alloc& alloc) {
        Forward_iter it = begin;
        size_type i = size_type{};

        try {
            for (; i != n; ++i, ++it, ++dest) {
                std::allocator_traits<Alloc>::construct(alloc, std::addressof(*dest), std::move(*it));
            }

        } catch (...) {
            --it;
            --dest;
            for (; i-- > 0; --it, --dest) {
                *it = std::move(*dest);
                std::allocator_traits<Alloc>::destroy(alloc, std::addressof(*begin));
            }

            throw;
        }
    }

    ///
    /// Allocator-aware version of std::uninitialized_copy
    ///
    template<class Input_iter, class Forward_iter, class Alloc>
    void uninitialized_copy(Input_iter begin, Input_iter end, Forward_iter dest, Alloc& alloc) {
        Forward_iter x = dest;

        try {
            for (Input_iter it = begin; it < end; ++it, ++x) {
                std::allocator_traits<Alloc>::construct(alloc, std::addressof(*x), *it);
            }

        } catch (...) {
            aul::destroy_n(begin, x, alloc);

            throw;
        }
    }

    ///
    /// Allocator-aware version of std::uninitialized_copy_n
    ///
    template<class Input_iter, class Forward_iter, class size_type, class Alloc>
    void uninitialized_copy_n(Input_iter begin, size_type n, Forward_iter dest, Alloc& alloc) {
        Forward_iter x = dest;
        Forward_iter y = begin;
        size_type i = size_type{};

        try {
            for (; i != n;++i, ++x, ++y) {
                std::allocator_traits<Alloc>::construct(alloc, std::addressof(*x), *y);
            }
        } catch (...) {
            aul::destroy_n(begin, i, alloc);

            throw;
        }
    }


    ///
    /// Allocator-aware hybridization of std::uninitialized_fill and std::iota
    /// Provides the weak guarantee
    ///
    template<class Forward_iter, class T, class Alloc>
    void uninitialized_iota(Forward_iter begin, Forward_iter end, T val, Alloc& alloc) {
        Forward_iter x = begin;
        try {
            for (; x < end; ++x, ++val) {
                std::allocator_traits<Alloc>::construct(alloc, std::addressof(*x), val);
            }

        } catch (...) {
            aul::destroy(begin, x, alloc);

            throw;
        }
    }

    ///
    /// Counted version of aul::uninitialized_iota
    /// Provides the weak guarantee
    ///
    template<class Forward_iter, class T, class size_type, class Alloc>
    void uninitialized_iota_n(Forward_iter begin, size_type n, T val, Alloc& alloc) {
        Forward_iter x = begin;
        size_type i = size_type{};

        try {
            for (;i != n;++i, ++x, ++val) {
                std::allocator_traits<Alloc>::construct(std::addressof(*x), val);
            }
        } catch (...) {
            aul::destroy(begin, i, alloc);
            throw;
        }
    }

    //=====================================================
    // Utility templates
    //=====================================================

    ///
    /// Template which is used to determine whether an allocator has default
    /// types for a given type T. In practice this typically means using raw
    /// pointers.
    ///
    template<typename T, class Alloc>
    class Allocator_has_trivial_types {
    private:
        using alloc_traits = std::allocator_traits<T>;

        using value_type = typename alloc_traits::value_type;
        using default_value_type = T;

        using pointer = typename alloc_traits::pointer;
        using default_pointer = value_type *;

        using const_pointer = typename alloc_traits::const_pointer;
        using default_const_pointer = typename std::pointer_traits<pointer>::template rebind<const value_type>;

        using void_pointer = typename alloc_traits::void_pointer;
        using default_void_pointer = typename std::pointer_traits<pointer>::template rebind<void>;

        using const_void_pointer = typename alloc_traits::const_void_pointer;
        using default_const_void_pointer = typename std::pointer_traits<pointer>::template rebind<const void>;

        using difference_type = typename alloc_traits::difference_type;
        using default_difference_type = typename std::pointer_traits<pointer>::difference_type;

        using size_type = typename alloc_traits::size_type;
        using default_size_type = typename std::make_unsigned<difference_type>::type;

    public:
        static constexpr bool value =
            std::is_same<value_type,      default_value_type     >::value &&
            std::is_same<pointer,         default_pointer        >::value &&
            std::is_same<const_pointer,   default_const_pointer  >::value &&
            std::is_same<void_pointer,    default_void_pointer   >::value &&
            std::is_same<difference_type, default_difference_type>::value &&
            std::is_same<size_type,       default_size_type      >::value;
    };

    /// Allocator_types
    /// Meant to reduce template code bloat by allowing iterators to be reused
    /// if two different allocators share the same type aliases
    ///
    template<typename Alloc>
    class Allocator_types {
    public:
        using value_type = typename std::allocator_traits<Alloc>::value_type;
        using pointer = typename std::allocator_traits<Alloc>::pointer;
        using const_pointer = typename std::allocator_traits<Alloc>::const_pointer;
        using difference_type = typename std::allocator_traits<Alloc>::difference_type;

        template<class U>
        class rebind {
            using other = typename std::allocator_traits<Alloc>::template rebind_alloc<U>;
        };
    };

    template<class A>
    struct is_noexcept_movable : public std::bool_constant<
        std::allocator_traits<A>::propagate_on_container_move_assignment::value ||
        std::allocator_traits<A>::is_always_equal::value
    >{};

    template<class A>
    inline constexpr bool is_noexcept_movable_v = is_noexcept_movable<A>::value;

}

#endif

#ifndef AUL_MEMORY_HPP
#define AUL_MEMORY_HPP

#include <memory>
#include <vector>
#include <type_traits>

namespace aul {

    template<class Ptr>
    auto to_raw_pointer(Ptr p) {
        return p.operator->();
    }

    template<class T>
    auto to_raw_pointer(T* p) {
        return p;
    }

    ///
    /// Allocator extended version of std::destroy
    ///
    /// \tparam F_iter Forward iterator type
    /// \tparam Alloc Allocator type
    /// \param first Iterator to beginning of range
    /// \param last Iterator to end of range
    /// \param alloc Reference to allocator object to destroy objects with
    template<class F_iter, class Alloc>
    void destroy(F_iter first, F_iter last, Alloc& alloc) noexcept {
        for (; first != last; ++first) {
            std::allocator_traits<Alloc>::destroy(alloc, to_raw_pointer(first));
        }
    }

    namespace impl {

        template<class Iter, class size_type, class Alloc>
        void destroy_n(Iter begin, size_type n, Alloc& alloc, std::random_access_iterator_tag) noexcept{
            destroy(begin, begin + n, alloc);
        }


        template<class Iter, class size_type, class Alloc>
        void destroy_n(Iter begin, size_type n, Alloc& alloc, std::input_iterator_tag) noexcept{
            for (size_type i = size_type{}; i != n; ++i, ++begin) {
                std::allocator_traits<Alloc>::destroy(alloc, std::addressof(*begin));
            }
        }

    }

    ///
    /// \tparam F_iter Forward iterator type
    /// \tparam size_type Integral type
    /// \tparam Alloc Allocator type
    /// \param begin Iterator to beginning of range
    /// \param n Size of range
    /// \param alloc Allocator object to destroy objects with
    template<class F_iter, class size_type, class Alloc>
    void destroy_n(F_iter begin, size_type n, Alloc& alloc) noexcept{
        using tag = typename std::iterator_traits<F_iter>::iterator_category;
        impl::destroy_n(begin, n, alloc, tag{});
    }

    ///
    /// Allocator extended version of std::uninitialized_default_construct
    ///
    /// \tparam F_iter Pointer type
    /// \tparam Alloc Allocator type
    /// \param begin Iterator to beginning of range
    /// \param end Iterator to end of range
    /// \param alloc Reference to allocator object to construct objects with
    template<class F_iter, class Alloc>
    void default_construct(F_iter begin, F_iter end, Alloc& alloc) {
        F_iter p = begin;
        try {
            for (; p != end; ++p) {
                std::allocator_traits<Alloc>::construct(alloc, to_raw_pointer(p));
            }
        } catch (...) {
            aul::destroy(begin, p, alloc);
            throw;
        }
    }

    namespace impl {

        template<class F_iter, class size_type, class Alloc>
        void default_construct_n(F_iter begin, size_type n, Alloc& alloc, std::random_access_iterator_tag) {
            default_construct(begin, begin + n, alloc);
        }

        template<class F_iter, class size_type, class Alloc>
        void default_construct_n(F_iter begin, size_type n, Alloc& alloc, std::input_iterator_tag) {
            F_iter x = begin;
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

    }

    ///
    /// Element count version of aul::uninitialized_default_construct
    ///
    /// \tparam F_iter Forward iterator type
    /// \tparam size_type Integral type
    /// \tparam Alloc Allocator type
    /// \param begin Iterator to beginning of range
    /// \param n Number of elements to default construct
    /// \param alloc Reference to allocator object to construct objects with
    template<class F_iter, class size_type, class Alloc>
    void default_construct_n(F_iter begin, const size_type n, Alloc& alloc) {
        using tag = typename std::iterator_traits<F_iter>::iterator_category;
        impl::default_construct_n(begin, n, alloc, tag{});
    }

    ///
    /// Allocator extended version of std::uninitialized_fill
    ///
    /// \tparam F_iter Forward iterator type
    /// \tparam T Type of object to be copied
    /// \tparam Alloc Allocator type
    /// \param begin Iterator to beginning of range
    /// \param end Iterator to end of range
    /// \param value Object to construct copies of in range
    /// \param allocator Allocator object to construct objects with
    template<class F_iter, class T, class Alloc>
    void uninitialized_fill(F_iter begin, F_iter end, const T& value, Alloc allocator) {
        F_iter x = begin;

        try {
            for (; x != end; ++x) {
                std::allocator_traits<Alloc>::construct(allocator, std::addressof(*x), value);
            }
        } catch (...) {
            aul::destroy(begin, x, allocator);
            throw;
        }
    }

    namespace impl {


        template<class F_iter, class size_type, class T, class Alloc>
        void uninitialized_fill_n(F_iter begin, const size_type n, const T& value, Alloc& alloc, std::random_access_iterator_tag) {
            uninitialized_fill(begin, begin + n, value, alloc);
        }

        template<class F_iter, class size_type, class T, class Alloc>
        void uninitialized_fill_n(F_iter begin, const size_type n, const T& value, Alloc& alloc, std::input_iterator_tag) {
            F_iter x = begin;
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

    }

    ///
    /// \tparam F_iter Forward iterator type
    /// \tparam size_type Integral type
    /// \tparam T Type of object to be copied
    /// \tparam Alloc Allocator type
    /// \param begin Iterator to beginning of range
    /// \param n Number of elements to default construct
    /// \param value Object to construct copies of in range
    /// \param allocator Allocator object to construct objects with
    template<class F_iter, class size_type, class T, class Alloc>
    void uninitialized_fill_n(F_iter begin, const size_type n, const T& value, Alloc& alloc) {
        using tag = typename std::iterator_traits<F_iter>::iterator_category;
        impl::uninitialized_fill_n(begin, n, value, alloc, tag{});
    }

    ///
    /// Extended version of std::move where moved-from objects are destroyed
    /// after being moved.
    ///
    /// Does not support overlapping ranges.
    ///
    /// \tparam In_iter Input iterator type
    /// \tparam Out_iter Output iterator type
    /// \tparam Alloc Allocator type
    /// \param first Iterator to beginning of source range
    /// \param last Iterator to end of source range
    /// \param d_first Iterator to beginning of destination range
    /// \param alloc Reference to allocator object to destroy objects with
    template<class In_iter, class Out_iter, class Alloc>
    Out_iter destructive_move(In_iter first, In_iter last, Out_iter d_first, Alloc& alloc) {
        for (;first != last; first++, d_first++) {
            *d_first = std::move(*first);
            std::allocator_traits<Alloc>::destroy(
                alloc,
                to_raw_pointer(first)
            );
        }
        return d_first;
    }

    ///
    /// Allocator extended version of std::move that constructs elements.
    ///
    /// Does not support overlapping ranges.
    ///
    /// \tparam In_iter Input iterator type
    /// \tparam Out_iter Forward iterator type
    /// \tparam Alloc Allocator type
    /// \param begin Iterator to beginning of source range
    /// \param end Iterator to end of source range
    /// \param dest Iterator to beginning of destination range
    /// \param allocator
    template<class In_iter, class Out_iter, class Alloc>
    Out_iter uninitialized_move(In_iter begin, In_iter end, Out_iter dest, Alloc& allocator) {
        for (; begin != end; ++begin, ++dest) {
            std::allocator_traits<Alloc>::construct(allocator, std::addressof(*dest), std::move(*begin));
        }
        return dest;
    }

    namespace impl {

        template<class In_iter, class Out_iter, class size_type, class Alloc>
        Out_iter uninitialized_move_n(In_iter begin, const size_type n, Out_iter dest, Alloc& alloc, std::random_access_iterator_tag) {
            return uninitialized_move(begin, begin + n, dest, alloc);
        }

        template<class In_iter, class Out_iter, class size_type, class Alloc>
        Out_iter uninitialized_move_n(In_iter begin, const size_type n, Out_iter dest, Alloc& alloc, std::forward_iterator_tag) {
            size_type i = {};
            for (; i != n; ++i, ++begin, ++dest) {
                std::allocator_traits<Alloc>::construct(alloc, std::addressof(*dest), std::move(*begin));
            }
            return dest;
        }

    }

    template<class In_iter, class Out_iter, class size_type, class Alloc>
    Out_iter uninitialized_move_n(In_iter begin, const size_type n, Out_iter dest, Alloc& alloc) {
        using tag = typename std::iterator_traits<In_iter>::iterator_category;
        return impl::uninitialized_move_n(begin, n, dest, alloc, tag{});
    }

    ///
    /// Allocator extended version of std::move that constructs move-to elements
    /// and destroys moved-from elements.
    ///
    /// Does not support overlapping ranges.
    ///
    /// \tparam In_iter  Input range
    /// \tparam Out_iter Output range
    /// \tparam Alloc Allocator type
    /// \param first Iterator to beginning of range
    /// \param last Iterator to end of range
    /// \param d_first Iterator to beginning of destination range
    /// \param alloc Reference to allocator object used to construct and destroy
    ///     elements
    template<class In_iter, class Out_iter, class Alloc>
    Out_iter uninitialized_destructive_move(In_iter first, In_iter last, Out_iter d_first, Alloc& alloc) {
        for (;first != last; first++, d_first++) {
            std::allocator_traits<Alloc>::construct(
                alloc,
                std::addressof(*d_first),
                std::move(*first)
            );

            std::allocator_traits<Alloc>::destroy(
                alloc,
                std::addressof(*first)
            );
        }
        return d_first;
    }

    ///
    /// Moves elements from [first, last) to [ret, dest) range with objects
    /// being move-constructed in destination range.
    ///
    /// Source and destination ranges may not overlap.
    ///
    /// \tparam Bi_iter0 Bi-directional iterator type
    /// \tparam Bi_iter1 Bi-direction iterator type
    /// \tparam Alloc Allocator type
    /// \param first Iterator to beginning of source range
    /// \param last Iterator to end of source range
    /// \param d_last Iterator to end of destination range
    /// \param alloc Allocator object to construct new objects with
    /// \return Iterator to beginning of destination range
    template<class Bi_iter0, class Bi_iter1, class Alloc>
    Bi_iter1 uninitialized_move_backward(Bi_iter0 first, Bi_iter0 last, Bi_iter1 d_last, Alloc& alloc) {
        while (first != last) {
            std::allocator_traits<Alloc>::construct(
                alloc,
                std::addressof(*(--d_last)),
                std::move(*(--last))
            );
        }
        return d_last;
    }

    ///
    /// Moves elements from [first, last) to [ret, dest) range with moved-from
    /// objects being destroyed.
    ///
    /// Source and destination ranges may not overlap
    ///
    /// \tparam Bi_iter0 Bi-directional iterator type
    /// \tparam Bi_iter1 Bi-direction iterator type
    /// \tparam Alloc Allocator type
    /// \param first Iterator to beginning of source range
    /// \param last Iterator to end of source range
    /// \param d_last Iterator to end of destination range
    /// \param alloc Allocator object to destroy objects with
    /// \return Iterator to beginning of destination range
    template<class Bi_iter0, class Bi_iter1, class Alloc>
    Bi_iter1 destructive_move_backward(Bi_iter0 first, Bi_iter0 last, Bi_iter1 d_last, Alloc& alloc) noexcept {
        while (first != last) {
            *(--d_last) = std::move(*(--last));
            std::allocator_traits<Alloc>::destroy(
                alloc,
                std::addressof(*(--d_last))
            );
        }
        return d_last;
    }

    ///
    /// Moves elements from [first, last) to [ret, dest) range with objects
    /// being move-constructed in destination range and moved-from objects being
    /// destroyed.
    ///
    /// \tparam Bi_iter0 Bi-directional iterator type
    /// \tparam Bi_iter1 Bi-direction iterator type
    /// \tparam Alloc Allocator type
    /// \param first Iterator to beginning of source range
    /// \param last Iterator to end of source range
    /// \param d_last Iterator to end of destination range
    /// \param alloc Allocator object to destroy objects with
    /// \return Iterator to beginning of destination range
    template<class Bi_iter0, class Bi_iter1, class Alloc>
    Bi_iter1 uninitialized_destructive_move_backward(Bi_iter0 first, Bi_iter0 last, Bi_iter1 d_last, Alloc& alloc) {
        while (first != last) {
            std::allocator_traits<Alloc>::construct(
                alloc,
                std::addressof(*(--d_last)),
                std::move(*(--last))
            );

            std::allocator_traits<Alloc>::destroy(
                alloc,
                std::addressof(*(--d_last))
            );
        }
        return d_last;
    }

    ///
    /// Allocator-aware version of std::uninitialized_copy
    ///
    /// \tparam Input_iter Input iterator type
    /// \tparam Forward_iter Forward iterator type
    /// \tparam Alloc Allocator type
    /// \param begin Iterator to beginning of range
    /// \param end Iterator to end of range
    /// \param dest Iterator to beginning of destination range
    /// \param alloc Allocator object to construct objects with
    template<class Input_iter, class Forward_iter, class Alloc>
    Forward_iter uninitialized_copy(Input_iter begin, Input_iter end, Forward_iter dest, Alloc& alloc) {
        Forward_iter x = dest;
        Input_iter it = begin;

        try {
            for (; it < end; ++it, ++x) {
                std::allocator_traits<Alloc>::construct(alloc, std::addressof(*x), *it);
            }
        } catch (...) {
            aul::destroy(dest, x, alloc);
            throw;
        }
        return x;
    }

    namespace impl {

        template<class Input_iter, class Forward_iter, class size_type, class Alloc>
        Forward_iter uninitialized_copy_n(Input_iter begin, const size_type n, Forward_iter dest, Alloc& alloc, std::random_access_iterator_tag) {
            return uninitialized_copy(begin, begin + n, dest, alloc);
        }


        template<class Input_iter, class Forward_iter, class size_type, class Alloc>
        Forward_iter uninitialized_copy_n(Input_iter begin, const size_type n, Forward_iter dest, Alloc& alloc, std::input_iterator_tag) {
            Forward_iter x = dest;
            Forward_iter y = begin;
            size_type i = size_type{};

            try {
                for (; i != n; ++i, ++x, ++y) {
                    std::allocator_traits<Alloc>::construct(alloc, std::addressof(*x), *y);
                }
            } catch (...) {
                aul::destroy_n(begin, i, alloc);
                throw;
            }
            return x;
        }

    }

    ///
    /// Allocator-aware version of std::uninitialized_copy_n
    ///
    /// \tparam Input_iter Input iterator type
    /// \tparam Forward_iter Forward iterator type
    /// \tparam size_type Integral type
    /// \tparam Alloc Allocator type
    /// \param begin Iterator to beginning of source range
    /// \param n Size of source and destination range
    /// \param dest Iterator to beginning of destination range
    /// \param alloc Allocator object to construct objects with
    template<class Input_iter, class Forward_iter, class size_type, class Alloc>
    Forward_iter uninitialized_copy_n(Input_iter begin, const size_type n, Forward_iter dest, Alloc& alloc) {
        using tag = typename std::iterator_traits<Input_iter>::iterator_category;
        return impl::uninitialized_copy_n(begin, n, dest, alloc, tag{});
    }

    ///
    /// Allocator-aware hybridization of std::uninitialized_fill and std::iota
    /// Provides the weak guarantee
    ///
    /// \tparam Forward_iter Forward iterator type
    /// \tparam T Type to fill range with
    /// \tparam Alloc Allocator type
    /// \param begin Iterator to beginning of range
    /// \param end Iterator to beginning of range
    /// \param val Value to begin counting
    /// \param alloc Allocator to construct objects with
    template<class Forward_iter, class T, class Alloc>
    void uninitialized_iota(Forward_iter begin, Forward_iter end, const T val, Alloc& alloc) {
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

    namespace impl {

        template<class Iter, class T, class size_type, class Alloc>
        void uninitialized_iota_n(Iter begin, const size_type n, const T val, Alloc& alloc, std::random_access_iterator_tag) {
            uninitialized_iota(begin, begin + n, val, alloc);
        }

        template<class Iter, class T, class size_type, class Alloc>
        void uninitialized_iota_n(Iter begin, const size_type n, const T val, Alloc& alloc, std::input_iterator_tag) {
            Iter x = begin;
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

    }

    ///
    /// Alternative to aul::uninitialized_iota
    ///
    /// \tparam F_iter Forward iterator type
    /// \tparam T Type to fill range with
    /// \tparam size_type Integral type
    /// \tparam Alloc Allocator type
    /// \param begin Iterator to beginning of range
    /// \param n Size of range
    /// \param val Value to begin counting
    /// \param alloc Allocator to construct objects with
    template<class F_iter, class T, class size_type, class Alloc>
    void uninitialized_iota_n(F_iter begin, const size_type n, const T val, Alloc& alloc) {
        using tag = typename std::iterator_traits<F_iter>::iterator_category;
        impl::destroy_n(begin, n, alloc, tag{});
    }

    //=====================================================
    // Element manipulation methods
    //=====================================================

    ///
    /// Move elements left to uninitialized locations.
    /// Destination range may overlap with input range.
    ///
    /// \tparam R_iter Random access iterator type
    /// \tparam Alloc Allocator type
    /// \param a Iterator to location where first element should be moved to.
    /// Must be less than b.
    /// \param b Iterator to first element to move
    /// \param c Iterator to one past last element to move
    /// \param alloc Allocator to use to construct type
    template<class R_iter, class Alloc>
    R_iter uninitialized_move_elements_left(R_iter a, R_iter b, R_iter c, Alloc& alloc) {
        auto non_overlap = std::min(c - b, b - a);
        auto it0 = b + non_overlap;
        auto it1 = aul::uninitialized_move(b, it0, a, alloc);
        return std::move(it0, c, it1);
    }

    ///
    /// Source and destination range may overlap
    ///
    /// \tparam R_iter Random access iterator
    /// \tparam Alloc Allocator type
    /// \param a Iterator to beginning of destination range
    /// \param b Iterator to beginning of source range
    /// \param c Iterator to end of source range
    /// \param alloc Allocator object to construct destroy elements with
    template<class R_iter, class Alloc>
    void destructive_move_elements_left(R_iter a, R_iter b, R_iter c, Alloc& alloc) {
        auto overlap = std::max((c - b) - (b - a), {});
        auto it0 = b + overlap;
        auto it1 = std::move(b, it0, a);
        aul::destructive_move(it0, c, it1, alloc);
    }

    ///
    /// Source and destination range amy overlap
    ///
    /// \tparam R_iter Random access iterator
    /// \tparam Alloc Allocator type
    /// \param a Iterator to beginning of destination range
    /// \param b Iterator to beginning of source range
    /// \param c Iterator to end of source range
    /// \param alloc Allocator object to construct destroy elements with
    /// \return Iterator to end of destination range
    template<class R_iter, class Alloc>
    R_iter uninitialized_destructive_move_elements_left(R_iter a, R_iter b, R_iter c, Alloc& alloc) {
        using d_type = typename std::iterator_traits<R_iter>::difference_type;

        auto source_range_size = (c - b);
        auto destination_size = std::min(b - a, source_range_size);

        bool condition = (destination_size  > (source_range_size / 2));

        auto y = destination_size;
        if (condition) {
            y = source_range_size - destination_size;
        }

        using std::max;

        d_type move_count = max(source_range_size - 2 * y, {});
        d_type constructive_move_count = y;
        d_type destructive_move_count = y;
        d_type constructive_destructive_mode_count = 0;

        if (condition) {
            std::swap(move_count, constructive_destructive_mode_count);
        }

        auto it0 = b;
        auto it1 = it0 + constructive_move_count;
        auto it2 = it1 + move_count;
        auto it3 = it2 + constructive_destructive_mode_count;
        auto it4 = it3 + destructive_move_count;

        R_iter it;
        it = aul::uninitialized_move(it0, it1, a, alloc);
        it = std::move(it1, it2, it);
        it = aul::uninitialized_destructive_move(it2, it3, it, alloc);
        it = aul::destructive_move(it3, it4, it, alloc);

        return it;
    }

    ///
    ///
    ///
    /// \tparam R_iter Random access iterator type
    /// \tparam size_type Integral type
    /// \tparam Alloc Allocator type
    /// \param a Iterator to location where first element should be moved to
    /// \param b Iterator to first element to move
    /// \param c Iterator to one past last element to move
    /// \param alloc Allocator to use to construct type
    template<class R_iter, class Alloc>
    R_iter uninitialized_move_elements_right(R_iter a, R_iter b, R_iter c, Alloc& alloc) {
        auto non_overlap = std::min(c - b, b - a);

        auto it0 = aul::uninitialized_move_backward(b - non_overlap, b, c, alloc);
        auto it1 = std::move_backward(a, b - non_overlap, it0);
        return it1;
    }

    ///
    /// \tparam R_iter
    /// \tparam Alloc
    /// \param a
    /// \param b
    /// \param c
    /// \param alloc
    template<class R_iter, class Alloc>
    R_iter destructive_move_elements_right(R_iter a, R_iter b, R_iter c, Alloc& alloc) {
        auto non_overlap = std::min(c - b, b - a);

        auto it0 = std::move_backward(b - non_overlap, b, c);
        auto it1 = aul::destructive_move_backward(a, b - non_overlap, it0, alloc);
        return it1;
    }

    ///
    /// \tparam R_iter Random access iterator
    /// \tparam Alloc Allocator type
    /// \param a Iterator to beginning of source range
    /// \param b Iterator to end of source range
    /// \param c Iterator to end of destination range
    /// \param alloc Allocator object with which to construct and destroy objects
    /// \return Iterator to beginning of destination range
    template<class R_iter, class Alloc>
    R_iter uninitialized_destructive_move_elements_right(R_iter a, R_iter b, R_iter c, Alloc& alloc) {
        using d_type = typename std::iterator_traits<R_iter>::difference_type;

        auto source_range_size = (b - a);
        auto destination_size = std::min(source_range_size, c - b);

        bool condition = (destination_size  > (source_range_size / 2));

        auto y = destination_size;
        if (condition) {
            y = source_range_size - destination_size;
        }

        using std::max;

        d_type move_count = max(source_range_size - 2 * y, {});
        d_type constructive_move_count = y;
        d_type destructive_move_count = y;
        d_type constructive_destructive_mode_count = 0;

        if (condition) {
            std::swap(move_count, constructive_destructive_mode_count);
        }

        auto it0 = b;
        auto it1 = it0 - constructive_move_count;
        auto it2 = it1 - move_count;
        auto it3 = it2 - constructive_destructive_mode_count;
        auto it4 = it3 - destructive_move_count;

        R_iter it;
        it = aul::uninitialized_move_backward(it1, it0, c, alloc);
        it = std::move_backward(it2, it1, it);
        it = aul::uninitialized_destructive_move_backward(it3, it2, it, alloc);
        it = aul::destructive_move_backward(it4, it3, it, alloc);

        return it;
    }

    //=====================================================
    // Utility templates
    //=====================================================

    ///
    /// Template which is used to determine whether an allocator has default
    /// types for a given type T. In practice this typically means using raw
    /// pointers.
    ///
    template<class T, class A>
    class allocator_has_trivial_types {
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
    template<class Alloc>
    class allocator_types {
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

    template<class A>
    struct is_noexcept_swappable : public std::bool_constant<
        std::allocator_traits<A>::propagate_on_container_swap::value ||
        std::allocator_traits<A>::is_always_equal::value
    > {};

    template<class A>
    constexpr bool is_noexcept_swappable_v = is_noexcept_swappable<A>::value;

}

#endif

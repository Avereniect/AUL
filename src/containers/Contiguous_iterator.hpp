#ifndef AUL_CONTIGUOUS_ITERATOR_HPP
#define AUL_CONTIGUOUS_ITERATOR_HPP

#include <type_traits>
#include <iterator>

namespace aul {

    template<typename T, typename Alloc_types, bool is_const>
    class Contiguous_iterator {
    public:

        //-----------------------------------------------------
        // Type aliases
        //-----------------------------------------------------

        using value_type = typename Alloc_types::value_type;

        using difference_type = typename Alloc_types::difference_type;

        using pointer = typename std::conditional<is_const,
            typename Alloc_types::const_pointer,
            typename Alloc_types::pointer
        >::type;

        using reference = typename std::conditional<is_const,
            const value_type&,
            value_type&
        >::type;

        using iterator_category = std::random_access_iterator_tag;

        //-----------------------------------------------------
        // -ctors
        //-----------------------------------------------------

        explicit Contiguous_iterator(const pointer ptr = pointer{}) noexcept:
            p(ptr) {}

        Contiguous_iterator(const Contiguous_iterator& itr) noexcept:
            p(itr.p) {}

        Contiguous_iterator(Contiguous_iterator&& itr) noexcept:
            p(itr.p) {
            itr.p = pointer{};
        }

        //-----------------------------------------------------
        // Assignment operators
        //-----------------------------------------------------

        Contiguous_iterator& operator=(const Contiguous_iterator& itr) noexcept;

        Contiguous_iterator& operator=(Contiguous_iterator&& itr) noexcept;

        //-----------------------------------------------------
        // Arithmetic operators
        //-----------------------------------------------------

        friend Contiguous_iterator& operator+(const Contiguous_iterator itr, const difference_type x) noexcept;

        friend Contiguous_iterator& operator+(const difference_type x, const Contiguous_iterator itr) noexcept;

        friend Contiguous_iterator& operator-(const Contiguous_iterator itr, const difference_type x) noexcept;

        friend Contiguous_iterator& operator-(const difference_type x, const Contiguous_iterator itr) noexcept;

        //-----------------------------------------------------
        // Increment/Decrement operators
        //-----------------------------------------------------

        Contiguous_iterator& operator++() noexcept;

        Contiguous_iterator& operator++(int) noexcept;

        Contiguous_iterator& operator--() noexcept;

        Contiguous_iterator& operator--(int) noexcept;

        //-----------------------------------------------------
        // Dereference operators
        //-----------------------------------------------------

        reference operator*() {
            return *p;
        }

        reference operator[](const difference_type x) {
            return p[x];
        }

        pointer operator->() noexcept {
            return p;
        }

    private:
        pointer p;
    };

}

#endif //AUL_CONTIGUOUS_ITERATOR_HPP

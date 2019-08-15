#ifndef AUL_RANDOM_ACCESS_ITERATOR_HPP
#define AUL_RANDOM_ACCESS_ITERATOR_HPP

#include <type_traits>
#include <iterator>

namespace aul {

    template<typename Alloc_types, bool is_const>
    class Random_access_iterator {

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

        explicit Random_access_iterator(const pointer ptr = pointer{}) noexcept:
            p(ptr) {}

        Random_access_iterator(const Random_access_iterator& itr) noexcept:
            p(itr.p) {}

        Random_access_iterator(Random_access_iterator&& itr) noexcept:
            p(itr.p) {
            itr.p = pointer{};
        }

        //-----------------------------------------------------
        // Assignment operators
        //-----------------------------------------------------

        Random_access_iterator& operator=(const Random_access_iterator& itr) noexcept {
            p = itr.p;
            return *this;
        }

        Random_access_iterator& operator=(Random_access_iterator&& itr) noexcept {
            p = itr.p;
            itr.p = pointer();

            return *this;
        }

        Random_access_iterator& operator+=(const difference_type x) noexcept {
            p += x;
            return *this;
        }

        Random_access_iterator& operator-=(const difference_type x) noexcept {
            p -= x;
            return *this;
        }

        //-----------------------------------------------------
        // Arithmetic operators
        //-----------------------------------------------------

        friend Random_access_iterator operator+(const Random_access_iterator itr, const difference_type x) noexcept {
            return Random_access_iterator{itr.p + x};
        }

        friend Random_access_iterator operator+(const difference_type x, const Random_access_iterator itr) noexcept {
            return Random_access_iterator{itr.p + x};
        }

        friend Random_access_iterator operator-(const Random_access_iterator itr, const difference_type x) noexcept {
            return Random_access_iterator{itr.p - x};
        }

        difference_type operator-(const Random_access_iterator rhs) noexcept {
            return Random_access_iterator{p - rhs.p};
        }

        //-----------------------------------------------------
        // Comparison operators
        //
        // Note: All comparison operators assume Alloc_types::pointer
        // and Alloc_types::const_pointer have appropriate comparison
        // operators
        //
        // TODO: C++ 20 Implement spaceship operator
        // TODO: C++ 20 Check for comparable concept
        //-----------------------------------------------------

        bool operator==(const Random_access_iterator rhs) noexcept {
            return p == rhs.p;
        }

        bool operator!=(const Random_access_iterator rhs) noexcept {
            return p != rhs.p;
        }

        bool operator<=(const Random_access_iterator rhs) noexcept {
            return p <= rhs.p;
        }

        bool operator>=(const Random_access_iterator rhs) noexcept {
            return p >= rhs.p;
        }

        bool operator<(const Random_access_iterator rhs) noexcept {
            return p < rhs.p;
        }

        bool operator>(const Random_access_iterator rhs) noexcept {
            return p > rhs.p;
        }

        //-----------------------------------------------------
        // Increment/Decrement operators
        //-----------------------------------------------------

        Random_access_iterator operator++() noexcept {
            p += 1;
            return *this;
        }

        Random_access_iterator operator++(int) noexcept {
            auto temp = *this;
            p += 1;
            return temp;
        }

        Random_access_iterator operator--() noexcept {
            p -= 1;
            return *this;
        }

        Random_access_iterator operator--(int) noexcept {
            auto temp = *this;
            p -= 1;
            return temp;
        }

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

        operator Random_access_iterator<Alloc_types, true>() {
            return Random_access_iterator<Alloc_types, true>(p);
        }

    private:
        pointer p;

    };

}

#endif //AUL_RANDOM_ACCESS_ITERATOR_HPP

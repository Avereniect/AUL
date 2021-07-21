#ifndef AUL_RANDOM_ACCESS_ITERATOR_HPP
#define AUL_RANDOM_ACCESS_ITERATOR_HPP

#include <type_traits>
#include <iterator>

namespace aul {

    /// Random_access_iterator
    /// General-purpose random-access iterator suitable for containers with
    /// contiguous storage of elements.
    ///
    /// \tparam Alloc_types Type containing allocator_type aliases.
    /// \tparam is_const    Is const_iterator
    template<typename Alloc_types, bool is_const>
    class Random_access_iterator {
    public:

        //=================================================
        // Type aliases
        //=================================================

        using value_type = typename Alloc_types::value_type;
        using difference_type = typename Alloc_types::difference_type;

        using pointer = typename std::conditional_t<is_const,
            typename Alloc_types::const_pointer,
            typename Alloc_types::pointer
        >;

        using reference = typename std::conditional_t<is_const,
            const value_type&,
            value_type&
        >;

        using iterator_category = std::random_access_iterator_tag;

        //=================================================
        // -ctors
        //=================================================

        Random_access_iterator() = default;

        explicit Random_access_iterator(const pointer ptr = pointer{}) noexcept:
            p(ptr) {}

        Random_access_iterator(const Random_access_iterator& itr) = default;

        Random_access_iterator(Random_access_iterator&& itr) = default;

        ~Random_access_iterator() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Random_access_iterator& operator=(const Random_access_iterator& itr) = default;

        Random_access_iterator& operator=(Random_access_iterator&& itr) = default;
        
        Random_access_iterator& operator+=(const difference_type x) noexcept {
            p += x;
            return *this;
        }

        Random_access_iterator& operator-=(const difference_type x) noexcept {
            p -= x;
            return *this;
        }

        //=================================================
        // Arithmetic operators
        //=================================================

        [[nodiscard]]
        Random_access_iterator operator+(const difference_type x) const {
            return Random_access_iterator{p + x};
        }

        [[nodiscard]]
        friend Random_access_iterator operator+(const difference_type x, const Random_access_iterator itr) {
            return Random_access_iterator{itr.p + x};
        }

        [[nodiscard]]
        Random_access_iterator operator-(const difference_type x) const {
            return Random_access_iterator{p - x};
        }

        [[nodiscard]]
        difference_type operator-(const Random_access_iterator rhs) const {
            return p - rhs.p;
        }

        //=================================================
        // Comparison operators
        //
        // Note: All comparison operators assume Alloc_types::pointer
        // and Alloc_types::const_pointer have appropriate comparison
        // operators
        //
        // TODO: C++ 20 Implement spaceship operator
        // TODO: C++ 20 Check for comparable concept
        //=================================================

        [[nodiscard]]
        bool operator==(const Random_access_iterator rhs) const {
            return p == rhs.p;
        }

        [[nodiscard]]
        bool operator!=(const Random_access_iterator rhs) const {
            return p != rhs.p;
        }

        [[nodiscard]]
        bool operator<=(const Random_access_iterator rhs) const {
            return p <= rhs.p;
        }

        [[nodiscard]]
        bool operator>=(const Random_access_iterator rhs) const {
            return p >= rhs.p;
        }

        [[nodiscard]]
        bool operator<(const Random_access_iterator rhs) const {
            return p < rhs.p;
        }

        [[nodiscard]]
        bool operator>(const Random_access_iterator rhs) const {
            return p > rhs.p;
        }

        //=================================================
        // Increment/Decrement operators
        //=================================================

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

        //=================================================
        // Dereference operators
        //=================================================

        [[nodiscard]]
        reference operator*() const {
            return *p;
        }

        [[nodiscard]]
        reference operator[](const difference_type x) const {
            return p[x];
        }

        [[nodiscard]]
        pointer operator->() const {
            return p;
        }

        //=================================================
        // Conversion operators
        //=================================================

        operator Random_access_iterator<Alloc_types, true>() {
            return Random_access_iterator<Alloc_types, true>(p);
        }

    private:
        pointer p;

    };

}

#endif //AUL_RANDOM_ACCESS_ITERATOR_HPP

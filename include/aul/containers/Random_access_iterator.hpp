#ifndef AUL_RANDOM_ACCESS_ITERATOR_HPP
#define AUL_RANDOM_ACCESS_ITERATOR_HPP

#include <type_traits>
#include <iterator>

namespace aul {

    /// Random_access_iterator
    /// General-purpose random-access iterator suitable for containers with
    /// contiguous storage of elements.
    ///
    /// \tparam P Pointer type
    template<class P>
    class Random_access_iterator {
    public:

        //=================================================
        // Type aliases
        //=================================================

        using value_type = typename std::pointer_traits<P>::element_type;
        using difference_type = typename std::pointer_traits<P>::difference_type;
        using pointer = P;
        using reference = value_type&;
        using iterator_category = std::random_access_iterator_tag;

        //=================================================
        // -ctors
        //=================================================

        explicit Random_access_iterator(const pointer ptr) noexcept:
            p(ptr) {}

        Random_access_iterator() = default;
        Random_access_iterator(const Random_access_iterator& itr) = default;
        Random_access_iterator(Random_access_iterator&& itr) noexcept = default;
        ~Random_access_iterator() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Random_access_iterator& operator=(const Random_access_iterator& itr) = default;
        Random_access_iterator& operator=(Random_access_iterator&& itr) noexcept = default;
        
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
        // These comparisons assume that the underlying
        // pointer has the appropriate comparison operators
        // overloaded despite not being required for fancy
        // pointers. A conversion to a raw pointer may be
        // a reasonable fallback.
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

        ///
        /// \return Conversion from iterator to non-const to iterator to const
        operator Random_access_iterator<typename std::pointer_traits<P>::template rebind<std::add_const_t<value_type>>>() {
            return Random_access_iterator<typename std::pointer_traits<P>::template rebind<std::add_const_t<value_type>>>{p};
        }

    private:
        pointer p;

    };

}

#endif //AUL_RANDOM_ACCESS_ITERATOR_HPP

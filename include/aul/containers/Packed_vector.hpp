#ifndef AUL_PACKED_VECTOR_HPP
#define AUL_PACKED_VECTOR_HPP

#include "Allocator_aware_base.hpp"
#include "Random_access_iterator.hpp"
#include "../memory/Memory.hpp"
#include "../memory/Allocation.hpp"
#include "../Algorithms.hpp"

#include <memory>

namespace aul {

    ///
    /// \tparam T
    /// \tparam A
    template<class T, class A = std::allocator<T>>
    class Packed_vector : private Allocator_aware_base<A> {
    public:

        //=================================================
        // Type aliases
        //=================================================

        using value_type = T;

        using reference = T&;
        using const_reference = const T&;

        using pointer = typename std::allocator_traits<A>::pointer;
        using const_pointer = typename std::allocator_traits<A>::const_pointer;

        using size_type = typename std::allocator_traits<A>::size_type;
        using difference_type = typename std::allocator_traits<A>::difference_type;

        using iterator = Random_access_iterator<pointer>;
        using const_iterator = Random_access_iterator<const_pointer>;

        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        using allocator_type = A;

    private:

        using base = Allocator_aware_base<A>;
        using allocation_type = aul::Allocation<T, A>;
        using alloc_traits = std::allocator_traits<A>;

    public:

        //=================================================
        // -ctors
        //=================================================

        Packed_vector() noexcept(noexcept(allocator_type{})) = default;

        explicit Packed_vector(const allocator_type& a) noexcept:
            base{a} {}

        Packed_vector(size_type count, const T& value, const allocator_type& alloc = {}):
            base{alloc},
            allocation(allocate(count)) {

            auto allocator = get_allocator();
            aul::uninitialized_fill_n(allocation.data, count, value, allocator);
        }

        explicit Packed_vector(size_type count, const allocator_type& alloc = {}):
            base(alloc),
            allocation(allocate(count)) {

            auto allocator = get_allocator();
            aul::default_construct_n(allocation.ptr, count, allocator);
        }

        template<class It>
        Packed_vector(It begin, It end, const allocator_type& alloc = {}):
            base{alloc},
            allocation(allocate(end - begin)) {

            auto allocator = get_allocator();
            aul::uninitialized_copy(begin, end, allocation.ptr, allocator);
        }

        Packed_vector(const Packed_vector& rhs):
            Packed_vector{rhs.begin(), rhs.end(), alloc_traits::select_on_container_copy_construction(rhs.get_allocator())} {}

        Packed_vector(const Packed_vector& rhs, const allocator_type& a):
            Packed_vector{rhs.begin(), rhs.end(), a} {}

        Packed_vector(Packed_vector&& other) noexcept:
            // This should technically be a move, but the net effect is
            // guaranteed to be equivalent for STL compliant allocators
            base{other.get_allocator()},
            allocation{std::move(other.allocation)} {}

        Packed_vector(Packed_vector&& other, const allocator_type& a):
            base{a},
            allocation{other.get_allocator() == a ? std::move(other.allocation) : allocate(other.size())} {

            //Indicates that other's allocation has not been moved
            if (!other.empty()) {
                std::copy_n(other.allocation.ptr, other.size(), allocation.ptr);
            }
        }

        Packed_vector(std::initializer_list<T> list, const allocator_type& a = {}):
            Packed_vector(list.begin(), list.end(), a) {}

        ~Packed_vector() {
            clear();
        }

        //=================================================
        // Assignment operators
        //=================================================

        Packed_vector& operator=(const Packed_vector& rhs) {
            base::operator=(rhs);
            auto new_allocation = allocate(rhs.size());

            auto allocator = get_allocator();
            try {
                aul::uninitialized_copy_n(rhs.allocation.ptr, rhs.allocation.capacity, new_allocation.ptr, allocator);
            } catch(...) {
                deallocate(new_allocation);
                throw;
            }

            clear();
            allocation = std::move(new_allocation);

            return *this;
        }

        Packed_vector& operator=(Packed_vector&& rhs) noexcept(aul::is_noexcept_movable_v<A>) {
            base::operator=(std::forward(rhs));

            if constexpr (alloc_traits::is_always_equal) {
                allocation = std::move(rhs.allocation);
            } else {
                if (get_allocator() == rhs.get_allocator()) {
                    allocation = std::move(rhs.allocation);
                    return *this;
                }

                auto allocator = get_allocator();

                auto new_allocation = allocate(rhs.size());

                try {
                    aul::uninitialized_move_n(rhs.allocation.ptr, rhs.allocation.ptr, allocation.ptr, allocator);
                } catch(...) {
                    deallocate(new_allocation);
                    throw;
                }

                //Destroy moved-from elements and deallocate rhs allocation, leaving it in the default state.
                aul::destroy_n(rhs.allocation.ptr, rhs.allocation.capacity, allocator);
                deallocate(rhs.allocation);

                clear();

                allocation = std::move(new_allocation);
            }

            return *this;
        }

        Packed_vector& operator=(std::initializer_list<T> list) {
            auto new_allocation = allocate(list.size());

            auto allocator = get_allocator();
            try {
                aul::uninitialized_copy_n(allocation.ptr, list.size(), new_allocation, allocator);
            } catch(...) {
                deallocate(new_allocation);
                throw;
            }

            deallocate(allocation);
            allocation = std::move(new_allocation);
        }

        //=================================================
        // Mutators
        //=================================================

        void assign(size_type count, const T& value) {
            auto new_allocation = allocate(count);

            auto allocator = get_allocator();
            try {
                aul::uninitialized_fill_n(new_allocation.ptr, count, value, allocator);
            } catch(...) {
                deallocate(new_allocation);
                throw;
            }

            clear();
            allocation = std::move(new_allocation);
        }

        template<class It>
        void assign(It begin, It end) {
            auto new_allocation = allocate(end - begin);

            auto allocator = get_allocator();
            try {
                aul::uninitialized_copy_n(begin, end, new_allocation.ptr, allocator);
            } catch(...) {
                deallocate(new_allocation);
                throw;
            }

            clear();
            allocation = std::move(new_allocation);
        }

        void assign(std::initializer_list<T> list) {
            assign(list.begin(), list.end());
        }

        void clear() noexcept {
            deallocate(allocation);
        }

        void resize(size_type count) {
            if (size() < count) {
                auto new_allocation = allocate(count);

                auto allocator = get_allocator();

                aul::uninitialized_move_n(allocation.ptr, allocation.capacity, new_allocation.ptr, allocator);

                try {
                    aul::default_construct(new_allocation.ptr + allocation.capacity, new_allocation.ptr + count, allocator);
                } catch (...) {
                    std::move(new_allocation.ptr, new_allocation.ptr + allocation.capacity);
                    aul::destroy_n(new_allocation.ptr, new_allocation.capacity, allocator);
                    deallocate(new_allocation);
                    throw;
                }

                deallocate(allocation);
                allocation = std::move(new_allocation);
            }

            if (count < size()) {
                auto new_allocation = allocate(count);

                auto allocator = get_allocator();
                aul::uninitialized_move_n(allocation.ptr, count, new_allocation.ptr, allocator);

                deallocate(allocation);
                allocation = std::move(new_allocation);
            }
        }

        void resize(size_type count, const value_type& value) {
            if (size() < count) {
                auto new_allocation = allocate(count);

                auto allocator = get_allocator();

                aul::uninitialized_move_n(allocation.ptr, allocation.capacity, new_allocation.ptr, allocator);

                try {
                    aul::uninitialized_fill(new_allocation.ptr + allocation.capacity, new_allocation.ptr + count, value, allocator);
                } catch (...) {
                    std::move(new_allocation.ptr, new_allocation.ptr + allocation.capacity);
                    aul::destroy_n(new_allocation.ptr, new_allocation.capacity, allocator);
                    deallocate(new_allocation);
                    throw;
                }

                deallocate(allocation);
                allocation = std::move(new_allocation);
            }

            if (count < size()) {
                auto new_allocation = allocate(count);

                auto allocator = get_allocator();
                aul::uninitialized_move_n(allocation.ptr, count, new_allocation.ptr, allocator);

                deallocate(allocation);
                allocation = std::move(new_allocation);
            }
        }

        //=================================================
        // Element accessors
        //=================================================

        reference at(size_type i) {
            if (allocation.capacity <= i) {
                throw std::out_of_range("Index out of bounds in call to aul::Tight_vector::at");
            }

            return operator[](i);
        }

        const_reference at(size_type i) const {
            if (allocation.capacity <= i) {
                throw std::out_of_range("Index out of bounds in call to aul::Tight_vector::at");
            }

            return operator[](i);
        }

        reference operator[](size_type i) {
            return allocation.ptr[i];
        }

        const_reference operator[](size_type i) const {
            return allocation.ptr[i];
        }

        reference front() {
            return allocation.ptr[0];
        }

        const_reference front() const {
            return allocation.ptr[0];
        }

        reference back() {
            return allocation.ptr[size() - 1];
        }

        const_reference back() const {
            return allocation.ptr[size() - 1];
        }

        //=================================================
        // Accessors
        //=================================================

        allocator_type get_allocator() const noexcept {
            return base::get_allocator();
        }

        size_type size() const {
            return capacity();
        }

        size_type max_size() const {
            constexpr size_type size_type_max = std::numeric_limits<size_type>::max();

            auto allocator = get_allocator();
            constexpr size_type alloc_max = alloc_traits::max_size(allocator);
            return std::min(size_type_max, alloc_max);
        }

        size_type capacity() const {
            return allocation.capacity;
        }

        bool empty() const {
            return allocation.capacity == 0;
        }

        T* data() noexcept {
            return allocation.ptr;
        }

        const T* data() const noexcept {
            return allocation.ptr;
        }

        //=================================================
        // Iterator methods
        //=================================================

        iterator begin() {
            return iterator{allocation.ptr};
        }

        const_iterator begin() const {
            return const_iterator{allocation.ptr};
        }

        const_iterator cbegin() const {
            return const_cast<const Packed_vector&>(*this).begin();
        }

        iterator end() {
            return iterator{allocation.ptr + allocation.capacity};
        }

        const_iterator end() const {
            return const_iterator{allocation.ptr + allocation.capacity};
        }

        const_iterator cend() const {
            return const_cast<const Packed_vector&>(*this).end();
        }

        //=================================================
        // Instance members
        //=================================================

    private:

        allocation_type allocation{};

        //=================================================
        // Helper functions
        //=================================================

        void deallocate(allocation_type& a) {
            auto allocator = get_allocator();
            alloc_traits::deallocate(allocator, allocation.ptr, allocation.capacity);
            a = {};
        }

        allocation_type allocate(size_type n) {
            allocation_type ret;

            allocator_type allocator = get_allocator();

            try {
                allocation.ptr = alloc_traits::allocate(allocator, n);
            } catch (...) {
                alloc_traits::deallocate(allocator, allocation.ptr, allocation.capacity);
                throw;
            }

            return ret;
        }

    };

    template<class T, class A>
    bool operator==(const Packed_vector<T, A>& lhs, const Packed_vector<T, A>& rhs) {
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    template<class T, class A>
    bool operator!=(const Packed_vector<T, A>& lhs, const Packed_vector<T, A>& rhs) {
        return !(lhs == rhs);
    }

    template<class T, class A>
    bool operator<(const Packed_vector<T, A>& lhs, const Packed_vector<T, A>& rhs) {
        return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    template<class T, class A>
    bool operator>(const Packed_vector<T, A>& lhs, const Packed_vector<T, A>& rhs) {
        return std::lexicographical_compare(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
    }

    template<class T, class A>
    bool operator<=(const Packed_vector<T, A>& lhs, const Packed_vector<T, A>& rhs) {
        return !(lhs > rhs);
    }

    template<class T, class A>
    bool operator>=(const Packed_vector<T, A>& lhs, const Packed_vector<T, A>& rhs) {
        return !(lhs < rhs);
    }

}

#endif //AUL_PACKED_VECTOR_HPP

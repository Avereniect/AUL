//
// Created by avereniect on 12/6/21.
//

#ifndef AUL_ARRAY_SET_HPP
#define AUL_ARRAY_SET_HPP

#include "Allocator_aware_base.hpp"
#include "Random_access_iterator.hpp"

#include "../Algorithms.hpp"
#include "../memory/Allocation.hpp"
#include "../memory/Memory.hpp"

#include <functional>
#include <memory>

namespace aul {

    template<class P, class D = typename std::pointer_traits<P>::difference_type>
    class Array_set_iterator {
    public:

    private:

    };

    template<class T, class C = std::less<T>, class A = std::allocator<T>>
    class Array_set : public aul::Allocator_aware_base<A> {
        using base = aul::Allocator_aware_base<A>;
    public:

        //=================================================
        // Type aliases
        //=================================================

        using value_type = T;

        using pointer = typename std::allocator_traits<A>::pointer;
        using const_pointer = typename std::allocator_traits<A>::const_pointer;

        using size_type = typename std::allocator_traits<A>::size_type;
        using difference_type = typename std::allocator_traits<A>::difference_type;

        using reference = T&;
        using const_reference = const T&;

        using iterator = Random_access_iterator<const_pointer>;
        using const_iterator = Random_access_iterator<const_pointer>;

        using allocator_type = A;

        using value_compare = C;

    private:

        using allocation_type = Allocation<size_type, pointer>;

    public:

        //=================================================
        // -ctors
        //=================================================

        ///
        /// \param c Comparator object
        /// \param alloc Allocator object
        explicit Array_set(const C& c = {}, const allocator_type& alloc = {}):
            base(alloc),
            num_elements(0),
            comparator(c),
            allocation() {}

        ///
        /// \param alloc Allocator object
        explicit Array_set(const allocator_type& alloc):
            base(alloc),
            num_elements(0),
            comparator(),
            allocation() {}

        ///
        /// \tparam It Forward iterator type
        /// \param first Iterator to beginning of range of objects to copy
        /// \param last Iterator to end of range of objects to copy
        /// \param c Comparator object
        /// \param alloc Allocator object
        template<class It>
        Array_set(It first, It last, const C& c = {}, const allocator_type& alloc = {}):
            base(alloc),
            num_elements(0),
            comparator(c),
            allocation(allocate(last - first)) {

            insert(first, last);
        }

        Array_set(const Array_set& other);
        Array_set(const Array_set& other, const allocator_type& a);

        Array_set(const Array_set&& other);
        Array_set(const Array_set&& other, const allocator_type& a);

        ///
        /// \param list Initializer list of objects to copy
        /// \param c Comparator object
        /// \param allocator Allocator object
        Array_set(std::initializer_list<T> list, const C& c, const allocator_type& allocator):
            base(allocator),
            num_elements(list.size()),
            comparator(c),
            allocation(allocate(list.size())) {

            insert(list);
        }

        Array_set(std::initializer_list<T> list, const allocator_type& allocator):
            Array_set(list, C{}, allocator) {}

        ~Array_set() {
            clear();
        }

        //=================================================
        // Assignment operators
        //=================================================

        Array_set& operator=(const Array_set& set);

        Array_set& operator=(Array_set&& set);

        Array_set& operator=(std::initializer_list<T> list);

        //=================================================
        // Iterator methods
        //=================================================

        const_iterator begin() const {
            return iterator{allocation.ptr};
        }

        const_iterator cbegin() const {
            return begin();
        }

        const_iterator end() const {
            return iterator{allocation.ptr + allocation.capacity};
        }

        const_iterator cend() const {
            return end();
        }

        //=================================================
        // Mutators
        //=================================================

        ///
        /// Provides the strong exception guarantee
        ///
        /// \param n Number of elements to reserve space for
        void reserve(size_type n) {
            if (n <= allocation.capacity) {
                return;
            }

            //This can also throw, but no handling can be done here
            allocation_type new_allocation = allocate(n);

            try {
                aul::uninitialized_copy_n(allocation.ptr, num_elements, new_allocation.ptr);
            } catch(...) {
                deallocate(new_allocation);
                throw;
            }

            deallocate(allocation);
            allocation = std::move(new_allocation);
        }

        //=================================================
        // Element mutators
        //=================================================

        ///
        /// \param value New value to insert
        /// \return Pair consists of iterator to newly inserted element, and
        ///     boolean indicating whether the new value was successfully
        ///     inserted
        std::pair<iterator, bool> insert(const value_type& value) {
            iterator it = aul::binary_search(begin(), end(), value, comparator);

            bool found = (it != end() && compare_equal(*it, value));
            if (found) {
                return {it, false};
            }

            bool space_needed = (num_elements + 1 < allocation.capacity);
            if (!space_needed) {
                //Construct new element at end
                construct_element(allocation.ptr + num_elements, std::move(allocation.ptr[num_elements - 1]));

                //Move over all other elements
                std::move(it, end() - 1, it + 1);

                // Insert new element via assignment
                *it = value;

                num_elements += 1;
                return {it, true};
            }

            //Handle case where new allocation is necessary

            allocation_type new_allocation = allocate(grow_size(num_elements));

            allocator_type alloc = get_allocator();

            pointer new_elem_location = new_allocation.ptr + (it - begin());
            construct_element(new_elem_location, value)

            aul::uninitialized_move_n(allocation.ptr, (it - begin()), new_allocation.ptr, alloc);
            aul::uninitialized_move_n(allocation.ptr + (it - begin()), end - it(), new_elem_location + 1, alloc);
            aul::destroy_n(allocation.ptr, num_elements, alloc);

            deallocate(allocation);
            allocation = std::move(new_allocation);

            num_elements += 1;

            return {it, true};
        }


        std::pair<iterator, bool> insert(value_type&& value);

        std::pair<iterator, bool> insert(const_iterator hint, const value_type& value);
        std::pair<iterator, bool> insert(const_iterator hint, value_type&& value);

        template<class It>
        void insert(It b, It e);

        void insert(std::initializer_list<T> list) {
            insert(list.begin(), list.end());
        }

        iterator erase(iterator pos);

        iterator erase(iterator b, iterator e);

        size_type erase(const value_type& value);

        void clear() noexcept {
            pointer e = allocation.ptr + num_elements;
            for (pointer p = allocation.ptr; p < e; ++p) {
                destroy_element(p);
            }

            deallocate(allocation);
            num_elements = 0;
        }

        void swap(Array_set& other) noexcept {
            std::swap(comparator, other.comparator);
            std::swap(base{*this}, base{other});
            std::swap(allocation, allocation);
        }

        //=================================================
        // Element accessors
        //=================================================

        const_iterator find(const value_type& value) const {
            return aul::binary_search(begin(), end(), value, comparator);
        }

        template<class K>
        const_iterator find(const K& k) const {
            return aul::binary_search(begin(), end(), k, comparator);
        }

        bool contains(const value_type& value) const {
            if (empty()) {
                return false;
            }

            iterator it = find(value);
            return (it != end());
        }

        template<class K>
        bool contains(const K& k) const {
            if (empty()) {
                return false;
            }

            iterator it = find(k);
            return (it != end());
        }

        //=================================================
        // Accessors
        //=================================================

        [[nodiscard]]
        size_type capacity() const noexcept {
            return allocation.capacity;
        }

        [[nodiscard]]
        size_type size() const noexcept {
            return num_elements;
        }

        [[nodiscard]]
        size_type max_size() const noexcept {
            constexpr size_type size_type_max = std::numeric_limits<size_type>::max();
            size_type allocator_max = base::get_allocator().max_size();
            return std::min(size_type_max, allocator_max);
        }

        [[nodiscard]]
        bool empty() const noexcept {
            return (num_elements == 0);
        }

        [[nodiscard]]
        pointer data() noexcept {
            return allocation.ptr;
        }

        [[nodiscard]]
        pointer data() const noexcept {
            return allocation.ptr;
        }

        allocator_type allocator() const noexcept {
            return base::get_allocator();
        }

        value_compare value_comp() const noexcept {
            return comparator;
        }

    private:

        //=================================================
        // Instance members
        //=================================================

        C comparator{};

        size_type num_elements{};

        allocation_type allocation{};

        //=================================================
        // Helper functions
        //=================================================

        ///
        /// Compute new allocation size given current size, assuming that a
        /// single new element way added to the container
        ///
        /// \param n Current vector size
        /// \return New allocation size
        size_type grow_size(size_type n) {
            constexpr size_type max = std::numeric_limits<size_type>::max();
            size_type half_max = max >> 1;

            if (half_max < n) {
                return max;
            } else {
                return n + n;
            }
        }

        ///
        /// \param x First object to compare
        /// \param y Second object to compare
        /// \return True if both objects compare equal. False otherwise
        bool compare_equal(const_reference x, const_reference y) {
            return !(comparator(x, y) || comparator(x, y));
        }

        ///
        ///
        /// \param ptr Pointer to container element to destroy
        void destroy_element(pointer ptr) {
            allocator_type a = base::get_allocator();
            std::allocator_traits<A>::destroy(a, ptr);
        }

        ///
        /// \tparam Args New element constructor argument types
        /// \param ptr Pointer to location where new element should be constructed
        /// \param args New element constructor arguments
        template<class...Args>
        void construct_element(pointer ptr, Args&&...args) {
            allocator_type a = base::get_allocator();
            std::allocator_traits<A>::construct(a, ptr, std::forward<Args>(args)...);
        }

        ///
        /// \param n New allocation size
        /// \param allocator Allocator to create allocation with
        /// \return Newly created allocation
        static allocation_type allocate(size_type n, const allocator_type& allocator) {
            allocation_type ret;

            try {
                ret.capacity = n;
                ret.ptr = std::allocator_traits<A>::allocate(allocator, n);
            } catch (...) {
                ret = {};
                throw;
            }

            return ret;
        }

        ///
        /// Allocate memory using current this's allocator
        ///
        /// \param n New allocation size
        /// \return Newly created allocation
        allocation_type allocate(size_type n) {
            allocator_type allocator = base::get_allocator();
            return allocate(n, allocator);
        }

        ///
        /// \param allocation Allocation to deallocate
        /// \param allocator Allocator from which allocation was made or
        ///     compatible allocator object
        static void deallocate(allocation_type& allocation, allocator_type& allocator) {
            std::allocator_traits<A>::deallocate(
                allocator,
                allocation.ptr,
                allocation.capacity
            );
        }

        ///
        /// Deallocate previously allocated memory using current this's
        /// allocator
        ///
        /// \param allocation Allocation to deallocate
        void deallocate(allocation_type& allocation) {
            allocator_type allocator = base::get_allocator();
            deallocate(allocation, allocator);
        }

    };

}

#endif //AUL_ARRAY_SET_HPP

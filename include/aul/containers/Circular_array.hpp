//
// Created by avereniect on 5/10/20.
//

#ifndef AUL_CIRCULAR_ARRAY_HPP
#define AUL_CIRCULAR_ARRAY_HPP

#include <memory>
#include <tuple>
#include <stdexcept>
#include <initializer_list>
#include <limits>
#include <algorithm>
#include <iterator>

#include <aul/containers/Allocator_aware_base.hpp>
#include <aul/memory/Allocation.hpp>
#include <aul/Algorithms.hpp>
#include <aul/memory/Memory.hpp>

namespace aul {

    ///
    /// Class meant to be used as aul::Circular_array::iterator.
    ///
    /// \tparam P Pointer type
    template<class P>
    class Circular_array_iterator {
    public:

        //=================================================
        // Type aliases
        //=================================================

        using value_type = typename std::iterator_traits<P>::value_type;
        using difference_type = typename std::iterator_traits<P>::difference_type;
        using reference = value_type&;
        using pointer = P;
        using iterator_category = std::random_access_iterator_tag;

        //=================================================
        // -ctors
        //=================================================

        Circular_array_iterator(const difference_type offset, pointer a, pointer b):
            offset(offset),
            begin(a),
            end(b) {}

        Circular_array_iterator() = default;
        Circular_array_iterator(const Circular_array_iterator& it) = default;
        Circular_array_iterator(Circular_array_iterator&& it) noexcept = default;
        ~Circular_array_iterator() = default;

        //=================================================
        // Assignment operators/methods
        //=================================================

        Circular_array_iterator& operator=(const Circular_array_iterator& it) = default;
        Circular_array_iterator& operator=(Circular_array_iterator&& it) noexcept = default;

        //=================================================
        // Comparison operators
        //=================================================

        [[nodiscard]]
        bool operator==(const Circular_array_iterator it) const {
            return (offset == it.offset) && (begin == it.begin) && (end == it.end);
        }

        [[nodiscard]]
        bool operator!=(const Circular_array_iterator it) const {
            return (offset != it.offset) || (begin != it.begin) || (end != it.end);
        }

        [[nodiscard]]
        bool operator<(const Circular_array_iterator it) const {
            return offset < it.offset;
        }

        [[nodiscard]]
        bool operator>(const Circular_array_iterator it) const {
            return offset > it.offset;
        }

        [[nodiscard]]
        bool operator<=(const Circular_array_iterator it) const {
            return offset <= it.offset;
        }

        [[nodiscard]]
        bool operator>=(const Circular_array_iterator it) const {
            return offset >= it.offset;
        }

        //=================================================
        // Increment/Decrement operators
        //=================================================

        Circular_array_iterator operator++() {
            ++offset;
            return *this;
        }

        Circular_array_iterator operator++(int) {
            auto temp = *this;
            ++offset;
            return temp;
        }

        Circular_array_iterator operator--() {
            --offset;
            return *this;
        }

        Circular_array_iterator operator--(int) {
            auto temp = *this;
            --offset;
            return temp;
        }

        //=================================================
        // Arithmetic operators
        //=================================================

        [[nodiscard]]
        Circular_array_iterator operator+(const difference_type x) const {
            auto temp = *this;
            temp.offset += x;
            return temp;
        }

        [[nodiscard]]
        Circular_array_iterator operator-(const difference_type x) const {
            auto temp = *this;
            temp.offset -= x;
            return temp;
        }

        [[nodiscard]]
        friend Circular_array_iterator operator+(const difference_type x, const Circular_array_iterator it) {
            it.offset += x;
            return it;
        }

        [[nodiscard]]
        difference_type operator-(const Circular_array_iterator it) const {
            return offset - it.offset;
        }

        //=================================================
        // Arithmetic assignment operators
        //=================================================

        [[nodiscard]]
        Circular_array_iterator operator+=(const difference_type x) {
            offset += x;
            return *this;
        }

        [[nodiscard]]
        Circular_array_iterator operator-=(const difference_type x) {
            offset -= x;
            return *this;
        }

        //=================================================
        // Dereference operators
        //=================================================

        [[nodiscard]]
        reference operator*() const {
            return *operator->();
        }

        [[nodiscard]]
        reference operator[](const difference_type x) const {
            return *(*this + x);
        }

        [[nodiscard]]
        pointer operator->() const {
            return (offset < 0 ? end  : begin) + offset;
        }

        //=================================================
        // Conversion operators
        //=================================================

        ///
        /// Implicit conversion from iterator from non-const to iterator to
        /// const
        ///
        /// \return Iterator to const which points to same location as this object
        [[nodiscard]]
        operator Circular_array_iterator<typename std::pointer_traits<P>::template rebind<const value_type>>() const {
            return {offset, begin, end};
        }

    private:

        //=================================================
        // Instance members
        //=================================================

        difference_type offset{};
        pointer begin{};
        pointer end{};

    };

    ///
    /// A vector-like container which allows for unused space at both before and
    /// after the elements in the allocation, potentially making insertions
    /// faster compared to std::vector, particularly near the start of the
    /// container.
    ///
    /// \tparam T Element type
    /// \tparam A Allocator type
    template<class T, class A = std::allocator<T>>
    class Circular_array : public aul::Allocator_aware_base<A> {
        using base = aul::Allocator_aware_base<A>;
    public:

        //=================================================
        // Type aliases
        //=================================================

        using allocator_type = A;

        using value_type = T;

        using reference = T&;
        using const_reference = const T&;

        using size_type = typename std::allocator_traits<allocator_type>::size_type;
        using difference_type = typename std::allocator_traits<allocator_type>::difference_type;

        using pointer = typename std::allocator_traits<A>::pointer;
        using const_pointer = typename std::allocator_traits<A>::const_pointer;

        using iterator = Circular_array_iterator<pointer>;
        using const_iterator = Circular_array_iterator<pointer>;

        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    private:

        using allocation_type = aul::Allocation<value_type, allocator_type>;

    public:

        //=================================================
        // -ctors
        //=================================================

        ///
        /// Constructs container holding no elements
        ///
        Circular_array() noexcept(noexcept(A{}))= default;

        ///
        /// Constructs a new Circular_array which uses a copy of the passed in
        /// object as its allocator.
        ///
        /// \param allocator Allocator to copy for internal use
        explicit Circular_array(allocator_type& a):
            base(a) {}

        ///
        /// Copy constructor
        ///
        /// Constructs a new Circular_array whose held elements are identical to
        /// those contained by the source object.
        ///
        /// \param arr Source object to copy from
        Circular_array(const Circular_array& arr):
            base(std::allocator_traits<A>::select_on_container_copy_construction(arr.get_allocator())),
            allocation(allocate(arr.size())),
            elem_count(arr.elem_count) {

            auto allocator = get_allocator();
            aul::uninitialized_copy(arr.cbegin(), arr.cend(), allocation.ptr, allocator);
        }

        ///
        /// Allocator extended copy constructor
        ///
        ///
        /// \param arr   Source object to copy from
        /// \param alloc Allocator copy should use
        Circular_array(const Circular_array& arr, const A& alloc):
            base(alloc),
            allocation(allocate(arr.size())),
            elem_count(arr.elem_count) {

            auto allocator = get_allocator();
            aul::uninitialized_copy(arr.cbegin(), arr.cend(), allocation.ptr, allocator);
        }

        ///
        /// Move constructor
        /// \param arr T Object to move resources from
        Circular_array(Circular_array&& arr) noexcept:
            base(std::move(arr.allocator)),
            allocation(std::move(allocation)),
            elem_count(arr.elem_count),
            head_offset(arr.head_offset) {

            arr.elem_count = 0;
            arr.head_offset = 0;
        }

        /// Allocator extended move constructor
        /// \param arr   Object to move resources from
        /// \param alloc Allocator container should copy
        Circular_array(Circular_array&& arr, const A& alloc):
            base(alloc),
            allocation(get_allocator() == arr.allocator ? std::move(arr.allocation) : allocate(arr.size())),
            elem_count(arr.elem_count),
            head_offset(get_allocator() == arr.allocator ? arr.head_offset : 0) {

            if (get_allocator() == arr.allocator) {
                arr.elem_count = 0;
                arr.head_offset = 0;
            } else {
                auto allocator = get_allocator();
                aul::uninitialized_move(arr.begin(), arr.end(), allocation.ptr, allocator);
            }
        }

        ///
        /// \param n     Number of elements to default constructor
        /// \param alloc Allocator containers should copy
        explicit Circular_array(const size_type n, const A& alloc = {}):
            base(alloc),
            allocation(allocate(n)),
            elem_count(n) {

            auto allocator = get_allocator();
            aul::default_construct(allocation.ptr, allocation.ptr + n, allocator);
        }

        ///
        /// \param n     Number of elements to construct
        /// \param val   Object elements should copy from
        /// \param alloc Allocator container should use
        Circular_array(const size_type n, const T& val, const A& alloc = {}):
            base(alloc),
            allocation(allocate(n)),
            elem_count(n) {

            auto allocator = get_allocator();
            aul::uninitialized_fill(allocation.ptr, allocation.ptr + n, val, allocator);
        }

        ///
        /// \param list  List to copy elements from
        /// \param alloc Allocator container should use
        Circular_array(const std::initializer_list<T>& list, const A& alloc = {}):
            Circular_array(list.begin(), list.end(), alloc) {}

        ///
        /// \tparam Iter Forward iterator type
        /// \param from Iterator to beginning of range to copy
        /// \param to Iterator to end of range to copy
        /// \param alloc Allocator for container to copy
        template<class Iter>
        Circular_array(Iter from, Iter to, const A& alloc = {}):
            base(alloc),
            allocation(allocate(std::distance(from, to))),
            elem_count(std::distance(from, to)) {

            auto allocator = get_allocator();
            aul::uninitialized_copy(from, to, allocation.ptr, allocator);
        }

        ///
        /// Destroys all elements contained within the container and frees any
        /// held allocation
        ///
        ~Circular_array() {
            clear();
            deallocate(allocation);
        }

        //=================================================
        // Assignment operators/methods
        //=================================================

        ///
        /// Copy assignment operator
        ///
        /// Invalidates iterators
        ///
        /// \param rhs Object to copy resources from
        /// \return *this
        Circular_array& operator=(const Circular_array& rhs) {
            Circular_array temp{rhs};
           this->swap(temp);

            return *this;
        }

        ///
        /// Move assignment operator
        ///
        /// Invalidates iterators
        ///
        /// \param rhs Object to move resources from
        /// \return *this
        Circular_array& operator=(Circular_array&& rhs) noexcept(aul::is_noexcept_movable<A>::value) {
            constexpr bool should_propagate =
                std::allocator_traits<A>::is_always_equal::value ||
                std::allocator_traits<A>::propagate_on_container_move_assignment::value;

            if (should_propagate) {
                base::operator=(rhs);
                allocation = std::move(rhs.allocation);
                elem_count = std::exchange(rhs.elem_count, 0);
                head_offset = std::exchange(rhs.head_offset, 0);
            } else {
                base::operator=(rhs);

                auto new_allocation = allocate(rhs.elem_count);
                auto allocator = get_allocator();
                aul::uninitialized_move(rhs.begin(), rhs.end(), new_allocation.ptr, allocator);

                rhs.deallocate(rhs.allocation);
                head_offset = 0;
                rhs.head_offset = 0;
                elem_count = std::exchange(rhs.elem_count, 0);
            }
            return *this;
        }

        ///
        /// Replaces the current contents of the container with copies of those
        /// in the specified list
        ///
        /// Provides the strong exception guarantee.
        ///
        /// Invalidates iterators.
        ///
        /// \param list List to copy elements from
        /// \return *this
        Circular_array& operator=(std::initializer_list<T> list) {
            assign(list);
            return *this;
        }

        ///
        /// Replaces the current contents of the container with copies of the
        /// elements in the range of [a, b].
        ///
        /// Provides the strong exception guarantee.
        ///
        /// Invalidates iterators.
        ///
        /// \tparam Iter Forward iterator type
        /// \param a Iterator to beginning of range
        /// \param b Iterator to end of range
        template<class Iter>
        void assign(Iter a, Iter b) {
            auto range_size = std::distance(a, b);

            if (range_size > max_size()) {
                throw std::length_error("aul::Circular_array grew beyond max size");
            }

            auto new_allocation = allocate(range_size);

            auto allocator = get_allocator();
            try {
                aul::uninitialized_copy(a, b, new_allocation.ptr, allocator);
            } catch (...) {
                deallocate(new_allocation);
                throw;
            }

            aul::destroy(begin(), end(), allocator);
            deallocate(allocation);
            allocation = new_allocation;
            elem_count = range_size;
            head_offset = 0;
        }

        ///
        /// Replace the current contents of the container with copies of the
        /// elements in list.
        ///
        /// Provides the strong exception guarantee.
        ///
        /// Invalidates iterators.
        ///
        /// \param list List to copy elements from
        void assign(const std::initializer_list<T> list) {
            assign(list.begin(), list.end());
        }

        ///
        /// Replace the current contents of the container with n copies of n.
        ///
        /// Provides the strong exception guarantee.
        ///
        /// Invalidates iterators.
        ///
        /// \param n   Number of elements to fill container with
        /// \param val Value to fill container with
        void assign(const size_type n, const T& val) {
            auto allocator = get_allocator();
            auto new_allocation = allocate(n);

            try {
                aul::uninitialized_fill_n(new_allocation.ptr, n, val, allocator);
            } catch (...) {
                deallocate(new_allocation);
                throw;
            }

            aul::destroy(begin(), end(), allocator);
            deallocate(allocation);
            elem_count = n;
            allocation = std::move(new_allocation);
            head_offset = 0;
        }

        //=================================================
        // Iterator methods
        //=================================================

        ///
        /// \return Iterator to beginning of element range.
        iterator begin() {
            bool c = is_segmented();
            return iterator{
                difference_type(head_offset - c * allocation.capacity),
                allocation.ptr,
                allocation.ptr + allocation.capacity
            };
        }

        ///
        /// \return Const iterator to end of element range.
        const_iterator begin() const {
            return const_cast<Circular_array&>(*this).begin();
        }

        ///
        /// \return Const iterator to beginning of element range.
        const_iterator cbegin() const {
            return begin();
        }

        ///
        /// \return Iterator to end of element range.
        iterator end() {
            bool c = is_segmented();
            return iterator{
                difference_type(head_offset - c * capacity() + size()),
                allocation.ptr,
                allocation.ptr + capacity()
            };
        }

        ///
        /// \return Const iterator to end of element range.
        const_iterator end() const {
            return const_cast<Circular_array&>(*this).end();
        }

        ///
        /// \return
        const_iterator cend() const {
            return end();
        }

        ///
        /// \return
        reverse_iterator rbegin() {
            return reverse_iterator{end()};
        }

        ///
        /// \return
        const_reverse_iterator rbegin() const {
            return const_reverse_iterator{end()};
        }

        ///
        /// \return
        const_reverse_iterator crbegin() const {
            return const_cast<const Circular_array&>(*this).rbegin();
        }

        ///
        /// \return
        reverse_iterator rend() {
            return reverse_iterator{begin()};
        }

        ///
        /// \return
        const_reverse_iterator rend() const {
            return const_reverse_iterator{begin()};
        }

        ///
        /// \return const reverse iterator to end of reverse range
        const_reverse_iterator crend() const {
            return const_cast<const Circular_array&>(*this).rend();
        }

        //=================================================
        // Element accessors
        //=================================================

        ///
        /// Undefined behavior if container is empty
        ///
        /// \return Reference to first element
        T& front() {
            return allocation.ptr[head_offset];
        }

        ///
        /// Undefined behavior if container is empty
        ///
        /// \return Const reference to first element
        const T& front() const {
            return allocation.ptr[head_offset];
        }

        ///
        /// Undefined behavior if container is empty
        ///
        /// \return Reference to last element
        T& back() {
            return *index_to_ptr(elem_count);
        }

        ///
        /// Undefined behavior if container is empty
        ///
        /// \return Const reference to last element
        const T& back() const {
            return *index_to_ptr(elem_count);
        }

        ///
        /// Undefined behavior if i >= size()
        ///
        /// \param i Element index
        /// \return Reference to element
        T& operator[](const size_type i) {
            return *index_to_ptr(i);
        }

        ///
        /// Undefined behavior if i >= size()
        ///
        /// \param i Element index
        /// \return Const reference to element
        const T& operator[](const size_type i) const {
            return *index_to_ptr(i);
        }

        ///
        /// Undefined behavior if i >= size()
        ///
        /// \param i Element index
        /// \return Reference to specified elements
        T& at(const size_type i) {
            if (size() <= i) {
                throw std::out_of_range("Circular_array::at() called with invalid index");
            }
            return *index_to_ptr(i);
        }

        ///
        /// \param i Element index
        /// \return Const reference to specified elements
        const T& at(const size_type i) const {
            if (size() <= i) {
                throw std::out_of_range("Circular_array::at() called with invalid index");
            }
            return *index_to_ptr(i);
        }

        //=================================================
        // Element addition
        //=================================================

        ///
        /// Constructs a new element as the specified position using the
        /// specified parameters.
        ///
        /// Provides the strong exception guarantee.
        ///
        /// \tparam Args Parameter types for new element's constructor
        /// \param p Iterator indicating position at which element should be
        ///     constructed
        /// \param args Parameters to new element's constructor
        template<class...Args>
        iterator emplace(const_iterator p, Args...args) {
            iterator it = begin() + (p - cbegin());

            if (elem_count < allocation.capacity) {
                return emplace_within_capacity(it, std::forward<Args>(args)...);
            } else {
                return emplace_with_new_allocation(it, std::forward<Args>(args)...);
            }
        }

        ///
        /// Copy-inserts the specified value at the specified position
        ///
        /// \param pos Position to insert new element at
        /// \param val Value to copy-insert
        /// \return Iterator to newly-inserted element
        iterator insert(const_iterator pos, const T& val) {
            return emplace(pos, val);
        }

        ///
        /// Move-inserts the specified value at the specified position
        ///
        /// \param pos Position to insert new elements at
        /// \param val Value to move-insert
        /// \return Iterator to newly-inserted element
        iterator insert(const_iterator pos, T&& val) {
            return emplace(pos, val);
        }

        ///
        /// Inserts multiple copies of the specified elements.
        ///
        /// \param pos Position at which new elements should be inserted
        /// \param n Number of copies of val to insert
        /// \param val Value to copy insert duplicates of
        /// \return Iterator to first of newly created elements
        iterator insert(const_iterator pos, const size_type n, const T& val) {
            iterator it = begin() + (pos - cbegin());

            //TODO: Prevent overflow
            if (elem_count + n < allocation.capacity) {
                return insert_within_capacity_n(it, n, val);
            } else {
                return insert_with_new_allocation_n(it, n, val);
            }
        }

        ///
        /// Performs copy-insertions of the elements specified in the list at
        /// the specified position.
        ///
        /// \tparam Iter Forward Iterator type
        /// \param pos Position to insert elements at
        /// \param from Iterator to beginning of range of elements to insert
        /// \param to Iterator to end of range of elements to insert
        /// \return Iterator to first of newly inserted elements
        template<class Iter>
        iterator insert(const_iterator pos, Iter from, Iter to) {
            auto d = to - from;

            if (max_size() - d < elem_count) {
                throw std::runtime_error("Circular_array grew beyond max size");
            }

            iterator it = begin() + (pos - cbegin());
            if (elem_count + d < capacity()) {
                return insert_within_capacity(it, from, to);
            } else {
                return insert_with_new_allocation(it, from, to);
            }
        }

        ///
        /// Performs copy-insertions of the elements specified in the list at
        /// the specified position.
        ///
        /// \param pos Position to insert new elements
        /// \param list List of elements to be copy-inserted
        /// \return Iterator to first of newly inserted elements
        iterator insert(const_iterator pos, const std::initializer_list<value_type>& list) {
            return insert(pos, list.begin(), list.end());
        }

        ///
        /// Constructs a new element as the new first element in the array using
        /// the specified parameters.
        ///
        /// Provides strong-exception guarantee.
        ///
        /// \tparam Args Types taken by object constructor
        /// \param args Arguments to constructor of new object
        template<class...Args>
        void emplace_front(Args...args) {
            if (elem_count > max_size() - 1) {
                throw std::length_error("aul::Circular_array grew beyond max size");
            }

            if (elem_count < allocation.capacity) {
                emplace_front_within_capacity(std::forward<Args>(args)...);
            } else {
                emplace_front_with_new_allocation(std::forward<Args>(args)...);
            }
        }

        ///
        /// Inserts a new element at the start of the logical array
        ///
        /// \param val Value to insert
        void push_front(const T& val) {
            emplace_front(val);
        }

        ///
        /// Inserts a new element at the start of the logical array
        ///
        /// \param val Value to insert
        void push_front(T&& val) {
            emplace_front(std::forward<T&&>(val));
        }

        ///
        /// Constructs a new element at the end of the logical array using the
        /// specified parameters.
        ///
        /// Provides strong-exception guarantee
        ///
        /// \tparam Args Parameter types for new element's constructor
        /// \param args Parameter types for new element's constructor
        template<class...Args>
        void emplace_back(Args...args) {
            if (size() > max_size() - 1) {
                throw std::length_error("aul::Circular_array grew beyond max size");
            }

            if (size() < capacity()) {
                emplace_back_within_capacity(std::forward<Args>(args)...);
            } else {
                emplace_back_with_new_allocation(std::forward<Args>(args)...);
            }
        }

        ///
        /// Inserts a new value at the end of the logical array
        ///
        /// \param val Value to insert
        void push_back(const T& val) {
            emplace_back(val);
        }

        ///
        /// Inserts a new value at the end of the logical array
        ///
        /// \param val Value to insert
        void push_back(T&& val) {
            emplace_back(std::forward<T&&>(val));
        }

        //=================================================
        // Element removal
        //=================================================

        ///
        /// Removes the first element in the container. Undefined behavior if
        /// container is empty.
        ///
        /// Invalidates iterators to first element
        ///
        void pop_front() {
            pointer ptr = begin().operator->();
            auto allocator = get_allocator();
            std::allocator_traits<A>::destroy(allocator, ptr);

            increment_head_offset();

            --elem_count;
        }

        ///
        /// Removes the last element in the container. Undefined behavior if
        /// container is empty.
        ///
        /// Invalidates iterators to last element
        ///
        void pop_back() {
            pointer ptr = (end() - 1).operator->();
            auto allocator = get_allocator();
            std::allocator_traits<A>::destroy(allocator, ptr);
            --elem_count;
        }

        ///
        /// Undefined behavior if pos is not a valid iterator
        ///
        /// Invalidates all iterators
        ///
        /// \param pos Iterator to element to remove.
        ///
        void erase(const_iterator pos) {
            size_type left = pos - begin();
            size_type right = cend() - pos - 1;

            if (left < right) {
                for (auto it = begin(); static_cast<const_iterator>(it) != pos; ++it) {
                    it[-1] = it[0];
                }
                auto allocator = get_allocator();
                std::allocator_traits<allocator_type>::destroy(allocator, begin().operator->());
            } else {
                for (auto it = end() - 1; it++ != it;) {
                    it[0] = it[1];
                }
                auto allocator = get_allocator();
                std::allocator_traits<allocator_type>::destroy(allocator, (end() - 1).operator->());
            }
            --elem_count;
        }

        ///
        /// Erase the specified range of elements.
        ///
        /// Invalidates iterators if from != to
        ///
        /// \param from Iterator to beginning of range
        /// \param to Iterator to end of range
        void erase(const_iterator from, const_iterator to);

        //=================================================
        // State Mutators
        //=================================================

        ///
        /// Ensure that the backing allocation holds enough space to store the
        /// specified number of elements
        ///
        /// Invalidates iterators
        ///
        /// \param n Number of elements to reserve space for
        void reserve(const size_type n) {
            if (n <= size()) {
                return;
            }

            if (max_size() < n) {
                throw std::length_error("aul::Circular_array::reserve() called with excessive allocation size");
            }

            auto allocator = get_allocator();
            allocation_type new_allocation = allocate(n);
            aul::uninitialized_move(begin(), end(), new_allocation.ptr, allocator);

            deallocate(allocation);
            allocation = new_allocation;
            head_offset = 0;
        }

        void swap(Circular_array& other) {
            base::swap(other);
            std::swap(allocation, other.allocation);
            std::swap(elem_count, other.elem_count);
            std::swap(head_offset, other.head_offset);
        }

        //=================================================
        // State accessors
        //=================================================

        ///
        ///
        /// \return True if the container does not hold any elements. False
        /// otherwise
        [[nodiscard]]
        bool empty() const {
            return elem_count == 0;
        }

        ///
        ///
        /// \return Number of elements which the container currently holds
        [[nodiscard]]
        size_type size() const {
            return elem_count;
        }

        ///
        ///
        /// \return Maximum size which the container can contain
        [[nodiscard]]
        size_type max_size() const {
            auto type_max = static_cast<size_type>(
                std::numeric_limits<difference_type>::max()
            );

            auto allocator = get_allocator();
            const size_type max_allocation = std::allocator_traits<A>::max_size(allocator);

            return std::min(max_allocation, type_max);
        }

        ///
        /// \return Number of elements which the container currently has space
        /// reserved for
        [[nodiscard]]
        size_type capacity() const {
            return allocation.capacity;
        }

        ///
        /// \return Copy of allocator used by container
        [[nodiscard]]
        allocator_type get_allocator() const {
            return base::get_allocator();
        }

        /*
        ///
        /// \return
        [[nodiscard]]
        std::tuple<pointer, pointer, pointer, pointer> data() {
            const auto segment0 = first_segment();
            const auto segment1 = second_segment();
            return {segment0.first, segment0.second, segment1.first, segment1.second};
        }

        ///
        /// \return
        [[nodiscard]]
        std::tuple<const_pointer, const_pointer, const_pointer, const_pointer> data() const {
            const auto segment0 = first_segment();
            const auto segment1 = second_segment();
            return {segment0.first, segment0.second, segment1.first, segment1.second};
        }
        */

        //=================================================
        // Misc. methods
        //=================================================

        ///
        /// Destroy all elements.
        ///
        void clear() {
            if (empty()) {
                return;
            }

            auto allocator = get_allocator();
            aul::destroy(begin(), end(), allocator);
            elem_count = 0;
            head_offset = 0;
        }

    private:

        //=================================================
        // Instance members
        //=================================================

        ///
        /// Currently held allocation
        ///
        Allocation<value_type, allocator_type> allocation{};

        ///
        /// Index of first element in the container
        ///
        difference_type head_offset{};

        ///
        /// Variable used to keep track of how many elements currently exist
        /// within the container
        ///
        size_type elem_count{};

        //=================================================
        // Helper functions
        //=================================================

        ///
        /// \param n Index of object to get pointer to
        /// \return Pointer to nth element
        pointer index_to_ptr(const size_type n) const {
            size_type index = 0;

            if (n < allocation.capacity - head_offset) {
                index = head_offset + n;
            } else {
                index = head_offset - allocation.capacity + n;
            }

            return allocation.ptr + index;
        }

        ///
        /// \param n Minimum number of elements to allocate storage for
        /// \return Size of new allocation
        size_type grow_size(size_type n) {
            if (n < max_size() / 2) {
                return std::max(2 * capacity(), n);
            } else {
                return max_size();
            }
        }

        //=================================================
        // Meta helpers
        //=================================================

        ///
        /// \return True if elements wrap around after reaching end of
        /// allocation.
        [[nodiscard]]
        bool is_segmented() const {
            auto s = static_cast<difference_type>(size());
            return s > (capacity() - head_offset);
        }

        /*
        ///
        /// \return
        [[nodiscard]]
        std::pair<pointer, pointer> first_segment() {
            if (is_segmented()) {
                return {allocation.ptr + head_offset, allocation.ptr + capacity()};
            }
            else {
                return {allocation.ptr + head_offset, allocation.ptr + head_offset + size()};
            };
        }

        [[nodiscard]]
        std::pair<const_pointer, const_pointer> first_segment() const {
            if (is_segmented()) {
                return {allocation.ptr + head_offset, allocation.ptr + capacity()};
            }
            else {
                return {allocation.ptr + head_offset, allocation.ptr + head_offset + size()};
            };
        }
        [[nodiscard]]
        std::pair<pointer, pointer> second_segment() {
            if (is_segmented()) {
                return {
                    allocation.ptr,
                    allocation.ptr + head_offset - capacity() + size()
                };
            } else {
                return {pointer{}, pointer{}};
            }
        }

        [[nodiscard]]
        std::pair<const_pointer, const_pointer> second_segment() const {
            if (is_segmented()) {
                return {
                    allocation.ptr,
                    allocation.ptr + head_offset - capacity() + size()
                };
            }
            else {
                return { pointer{}, pointer{} };
            }
        }
        */

        //=================================================
        // Allocation methods
        //=================================================

        [[nodiscard]]
        allocation_type allocate(const size_type n) {
            allocation_type alloc{};

            try {
                auto allocator = get_allocator();
                alloc.ptr = std::allocator_traits<allocator_type>::allocate(allocator, n);
                alloc.capacity = n;
            } catch (...) {
                alloc = {};
                throw;
            }

            return alloc;
        }

        [[nodiscard]]
        allocation_type allocate(const size_type n, const allocation_type& hint) {
            allocation_type alloc{};

            try {
                auto allocator = get_allocator();
                alloc.ptr = std::allocator_traits<allocator_type>::allocate(allocator, n, hint.ptr);
                alloc.capacity = n;
            } catch (...) {
                alloc = {};
                throw;
            }

            return alloc;
        }

        void deallocate(allocation_type& alloc) {
            auto allocator = get_allocator();
            std::allocator_traits<allocator_type>::deallocate(allocator, alloc.ptr, alloc.capacity);
            alloc = {};
        }

        //=================================================
        // Element construction/destruction
        //=================================================

        template<class Iter>
        iterator insert_within_capacity(iterator pos, Iter a, Iter b) {
            auto left  = (pos - begin());
            auto right = (end() - pos);

            if (left < right) {
                return insert_within_capacity_nudge_left(pos, a, b);
            } else {
                return insert_within_capacity_nudge_right(pos, a, b);
            }
        }

        template<class Iter>
        iterator insert_within_capacity_nudge_left(iterator pos, Iter a, Iter b) {
            //TODO: Implement
        }

        template<class Iter>
        iterator insert_within_capacity_nudge_right(iterator pos, Iter a, Iter b) {
            //TODO: Implement
        }

        template<class Iter>
        iterator insert_with_new_allocation(iterator pos, Iter a, Iter b) {
            auto allocator = get_allocator();
            auto d = (b - a);

            auto new_capacity = grow_size(elem_count + d);
            auto new_allocation = allocate(new_capacity);

            pointer p1{};
            auto o = pos - begin();
            try {
                auto p = new_allocation.ptr + o;
                p1 = aul::uninitialized_copy(a, b, p, allocator);
            } catch (...) {
                deallocate(new_allocation);
                throw;
            }

            aul::uninitialized_move(begin(), pos, new_allocation, allocator);
            aul::uninitialized_move(pos, end(), p1, allocator);

            aul::destroy(begin(), end(), allocator);
            deallocate(allocator);

            allocation = new_allocation;

            elem_count += d;
            head_offset = 0;

            return {o, allocation.ptr, allocation.ptr + allocation.capacity};
        }

        ///
        /// Construct a new element as the first element in the container under
        /// the assumption that size() < capacity()
        ///
        /// \tparam Args
        /// \param args
        template<class...Args>
        void emplace_front_within_capacity(Args&&...args) {
            ++elem_count;
            decrement_head_offset();
            pointer ptr = std::addressof(*begin());

            auto allocator = get_allocator();
            std::allocator_traits<allocator_type>::construct(
                allocator,
                ptr,
                std::forward<Args>(args)...
            );
        }

        template<class...Args>
        void emplace_front_with_new_allocation(Args&&...args) {
            auto new_capacity = grow_size(elem_count + 1);
            auto new_allocation = allocate(new_capacity);

            auto allocator = get_allocator();
            try {
                std::allocator_traits<allocator_type>::construct(
                    allocator,
                    new_allocation.ptr,
                    std::forward<Args>(args)...
                );
            } catch(...) {
                deallocate(new_allocation);
                throw;
            }

            aul::uninitialized_move(begin(), end(), new_allocation.ptr + 1, allocator);
            aul::destroy(begin(), end(), allocator);
            deallocate(allocation);
            allocation = new_allocation;

            ++elem_count;
            head_offset = 0;
        }

        ///
        ///
        ///
        /// \tparam Args
        /// \param args
        template<class...Args>
        void emplace_back_within_capacity(Args&&...args) {
            pointer ptr = index_to_ptr(elem_count);

            auto allocator = get_allocator();
            std::allocator_traits<allocator_type>::construct(
                allocator,
                ptr,
                std::forward<Args>(args)...
            );

            ++elem_count;
        }

        template<class...Args>
        void emplace_back_with_new_allocation(Args&&...args) {
            size_type new_capacity = grow_size(elem_count + 1);
            allocation_type new_allocation = allocate(new_capacity);

            allocator_type allocator = get_allocator();
            try {
                pointer p = new_allocation.ptr + elem_count;
                std::allocator_traits<allocator_type>::construct(
                    allocator,
                    p,
                    std::forward<Args>(args)...
                );
            } catch (...) {
                deallocate(new_allocation);
                throw;
            }

            aul::uninitialized_move(begin(), end(), new_allocation.ptr, allocator);
            aul::destroy(begin(), end(), allocator);
            deallocate(allocation);
            allocation = new_allocation;

            head_offset = 0;
            ++elem_count;
        }

        ///
        /// Emplaces a new element under the assumption that size() < capacity()
        ///
        /// Existing elements are moved left or right depending on which would
        /// require fewer elements to be moved
        ///
        /// \tparam Args Parameters types for new element's constructor
        /// \tparam Args Parameters types for new element's constructor
        template<class...Args>
        iterator emplace_within_capacity(iterator it, Args&&...args) {
            auto left = (it - begin());
            auto right = (end() - it);
            if (left < right) {
                return emplace_within_capacity_nudge_left(it, std::forward<Args>(args)...);
            } else {
                return emplace_within_capacity_nudge_right(it, std::forward<Args>(args)...);
            }
        }

        ///
        /// \tparam Args Parameters types for new element's constructor
        /// \param it Iterator to position where new element should be
        ///     constructed
        /// \param args Parameters for new element's constructor
        template<class...Args>
        iterator emplace_within_capacity_nudge_left(iterator it, Args&&...args) {
            auto allocator = get_allocator();

            auto it0 = begin() - 1;
            auto it1 = begin();
            auto it2 = it + 1;

            auto it3 = aul::uninitialized_destructive_move_elements_left(it0, it1, it2, allocator);

            pointer p = std::addressof(it[-1]);
            try {
                std::allocator_traits<allocator_type>::construct(
                    allocator,
                    p,
                    std::forward<Args>(args)...
                );
            } catch(...) {
                aul::uninitialized_destructive_move_elements_right(it0, it3, it, allocator);
                throw;
            }

            decrement_head_offset();
            ++elem_count;

            return it - 1;
        }

        ///
        /// \tparam Args Parameters types for new element's constructor
        /// \param it Iterator to position where new element should be
        ///     constructed
        /// \param args Parameters for new element's constructor
        template<class...Args>
        iterator emplace_within_capacity_nudge_right(iterator it, Args&&...args) {
            auto allocator = get_allocator();

            auto it0 = it;
            auto it1 = end();
            auto it2 = end() + 1;

            auto it3 = aul::uninitialized_destructive_move_elements_right(it0, it1, it2, allocator);

            pointer p = std::addressof(*it);
            try {
                std::allocator_traits<allocator_type>::construct(
                    allocator,
                    p,
                    std::forward<Args>(args)...
                );
            } catch(...) {
                aul::uninitialized_destructive_move_elements_left(it, it3, it2, allocator);
                throw;
            }

            ++elem_count;
            return it;
        }

        iterator insert_within_capacity_n(iterator it, size_type n, const T& val) {
            auto left = it - begin();
            auto right = end() - it;
            if (left < right) {
                return insert_within_capacity_nudge_left_n(it, n, val);
            } else {
                return insert_within_capacity_nudge_right_n(it, n, val);
            }
        }

        iterator insert_within_capacity_nudge_left_n(iterator it, size_type n, const T& val) {
            auto allocator = get_allocator();

            auto it0 = begin() - n;
            auto it1 = begin();
            auto it2 = it + 1;

            auto it3 = aul::uninitialized_destructive_move_elements_left(it0, it1, it2, allocator);

            try {
                aul::uninitialized_fill_n(it - n, n, val, allocator);
            } catch(...) {
                aul::uninitialized_destructive_move_elements_right(it0, it3, it, allocator);
                throw;
            }

            decrease_head_offset(n);
            elem_count += n;

            return (it - n);
        }

        iterator insert_within_capacity_nudge_right_n(iterator it, size_type n, const T& val) {
            auto allocator = get_allocator();

            auto it0 = it;
            auto it1 = end();
            auto it2 = end() + n;

            auto it3 = aul::uninitialized_destructive_move_elements_right(it0, it1, it2, allocator);

            try {
                aul::uninitialized_fill_n(it, n, val, allocator);
            } catch(...) {
                aul::uninitialized_destructive_move_elements_left(it, it3, it2, allocator);
                throw;
            }

            elem_count += n;
            return it;
        }

        iterator insert_with_new_allocation_n(iterator it, size_type n, const T& val) {
            allocation_type new_allocation = allocate(size() + n);

            auto allocator = get_allocator();

            pointer p = new_allocation.ptr + (it - begin());
            try {
                aul::uninitialized_fill_n(p, n, val, allocator);
            } catch (...) {
                deallocate(new_allocation);
                throw;
            }

            aul::uninitialized_move(begin(), it, allocation.ptr, allocator);
            aul::uninitialized_move(it, end(), p + n, allocator);
            elem_count += n;
            head_offset = 0;

            allocation = new_allocation;
            return {it - begin(), allocation.ptr, allocation.ptr + allocation.capacity};
        }

        ///
        /// Emplaces a new element within a new allocation, replacing the
        /// current allocation.
        ///
        /// \tparam Args Parameters types for new element's constructor
        /// \param it Iterator indicating position at which element should be
        ///     constructedc
        /// \param args Parameters for new element's constructor
        template<class...Args>
        iterator emplace_with_new_allocation(iterator it, Args&&...args) {
            size_type new_capacity = grow_size(elem_count + 1);
            allocation_type new_allocation = allocate(new_capacity);

            //The pointers in this iterator may need to be laundered
            iterator ret{
                it - begin(),
                new_allocation.ptr,
                new_allocation.ptr + new_allocation.capacity
            };

            auto allocator = get_allocator();
            pointer p = std::addressof(*ret);
            try {
                std::allocator_traits<allocator_type>::construct(
                    allocator,
                    p,
                    std::forward<Args>(args)...
                );
            } catch (...) {
                deallocate(new_allocation);
                throw;
            }

            //Move elements to left of new element into position
            aul::uninitialized_move(begin(), it, new_allocation.ptr, allocator);

            //Move elements to the right of new element into position
            aul::uninitialized_move(it, end(), p + 1, allocator);

            destroy(begin(), end(), allocator);
            deallocate(allocation);
            allocation = new_allocation;

            head_offset = 0;
            ++elem_count;

            return ret;
        }

        void increment_head_offset() {
            head_offset += 1;
            if (head_offset == size()) {
                head_offset = 0;
            }
        }

        void decrement_head_offset() {
            if (head_offset == 0) {
                head_offset = (allocation.capacity - 1);
            } else {
                --head_offset;
            }
        }

        void increase_head_offset(size_type d) {
            d %= elem_count;

            if (head_offset < elem_count - d) {
                head_offset += d;
            } else {
                head_offset = head_offset - size() + d;
            }
        }

        void decrease_head_offset(size_type d) {
            //TODO: Correct implementation
            d %= elem_count;

            if (head_offset > -elem_count + d) {
                head_offset -= d;
            } else {
                head_offset = head_offset + size() - d;
            }
        }

    };

}

#endif //AUL_CIRCULAR_ARRAY_HPP

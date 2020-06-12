//
// Created by avereniect on 5/10/20.
//

#ifndef AUL_CIRCULAR_ARRAY_HPP
#define AUL_CIRCULAR_ARRAY_HPP

#include "../Algorithms.hpp"
#include "../memory/Memory.hpp"

#include <memory>
#include <tuple>
#include <stdexcept>
#include <initializer_list>
#include <limits>

namespace aul {

    template<class T, class A = std::allocator<T>>
    class Circular_array {
    public:
        template<bool is_const>
        class Iterator;

    private:
        class Allocation;

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

        using iterator = Iterator<false>;
        using const_iterator = Iterator<true>;

        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        //=================================================
        // -ctors
        //=================================================

        ///
        /// Default constructor
        /// Constructs container holding no elements
        ///
        constexpr Circular_array() noexcept(noexcept(A{}))= default;

        /// Copy constructor
        /// \param arr Source object to copy from
        ///
        constexpr Circular_array(const Circular_array& arr):
            allocator(std::allocator_traits<A>::select_on_container_copy_construction(arr.allocator)),
            allocation(allocate(arr.size())),
            elem_count(arr.elem_count) {

            aul::uninitialized_copy(arr.cbegin(), arr.cend(), allocation.array, allocator);
        }

        /// Allocator extended copy constructor
        /// \param arr   Source object to copy from
        /// \param alloc Allocator copy should use
        constexpr Circular_array(const Circular_array& arr, const allocator_type& alloc):
            allocator(alloc),
            allocation(allocate(arr.size())),
            elem_count(arr.elem_count) {

            aul::uninitialized_copy(arr.cbegin(), arr.cend(), allocation.array, allocator);
        }

        ///
        /// Move constructor
        /// \param arr T Object to move resources from
        constexpr Circular_array(Circular_array&& arr) noexcept:
            allocator(std::move(arr.allocator)),
            allocation(std::move(allocation)),
            elem_count(arr.elem_count),
            head_offset(arr.head_offset) {

            arr.elem_count = 0;
            arr.head_offset = 0;
        }

        /// Allocator extended move constructor
        /// \param arr   Object to move resources from
        /// \param alloc Allocator container should copy
        constexpr Circular_array(Circular_array&& arr, const allocator_type& alloc):
            allocator(alloc),
            allocation(allocator == arr.allocator ? std::move(arr.allocation) : allocate(arr.size())),
            elem_count(arr.elem_count),
            head_offset(allocator == arr.allocator ? arr.head_offset : 0) {

            if (allocator == arr.allocator) {
                arr.elem_count = 0;
                arr.head_offset = 0;
            } else {
                aul::uninitialized_move(arr.begin(), arr.end(), allocation.array, allocator);
            }
        }

        ///
        /// \param n Number of elements to default construct
        ///
        constexpr Circular_array(const size_type n):
            allocator({}),
            allocation(allocate(n)),
            elem_count(n) {

            aul::default_construct(allocation.array, allocation.array + n, allocator);
        }

        ///
        /// \param n     Number of elements to default constructor
        /// \param alloc Allocator containers should copy
        constexpr Circular_array(const size_type n, const allocator_type& alloc):
            allocator(alloc),
            allocation(allocate(n)),
            elem_count(n) {

            aul::default_construct(allocation.array, allocation.array + n, allocator);
        }

        ///
        /// \param n   Number of elements to construct
        /// \param val Object elements should copy from
        constexpr Circular_array(const size_type n, const T& val):
            allocator({}),
            allocation(allocate(n)),
            elem_count(n) {

            aul::uninitialized_fill(allocation.array, allocation.array + n, val, allocator);
        }

        ///
        /// \param n     Number of elements to construct
        /// \param val   Object elements should copy from
        /// \param alloc Allocator container should use
        constexpr Circular_array(const size_type n, const T& val, const allocator_type& alloc):
            allocator(alloc),
            allocation(allocate(n)),
            elem_count(n) {

            aul::uninitialized_fill(allocation.array, allocation.array + n, val, allocator);
        }

        ///
        /// \param list List to copy elements from
        ///
        constexpr Circular_array(const std::initializer_list<T>& list):
            allocator({}),
            allocation(allocate(list.end() - list.begin())),
            elem_count(list.end() - list.begin()) {

            aul::uninitialized_copy(list.begin(), list.end(), allocation.array, allocator);
        }

        ///
        /// \param list  List to copy elements from
        /// \param alloc Allocator container should use
        constexpr Circular_array(const std::initializer_list<T>& list, const allocator_type& alloc):
            allocator(alloc),
            allocation(allocate(list.end() - list.begin())),
            elem_count(list.end() - list.begin()) {

            aul::uninitialized_copy(list.begin(), list.end(), allocation.array, allocator);
        }

        ///
        /// Destructor
        ///
        constexpr ~Circular_array() {
            clear();
        }

        //=================================================
        // Assignment operators/methods
        //=================================================

        /// Copy assignment operator
        /// \param rhs Object to copy resources from
        /// \return *this
        constexpr Circular_array& operator=(const Circular_array& rhs) {
            clear();

            //TODO: Provide strong exception gaurentee
            if constexpr (std::allocator_traits<A>::propagate_on_container_copy_assignment::value) {
                allocator = rhs.allocator;
            }

            allocation = allocate(rhs.size());
            head_offset = 0;
            elem_count = rhs.elem_count;

            const auto segment0 = rhs.first_segment();
            const auto segment1 = rhs.second_segment();

            const auto segment0_size = segment0.second - segment0.first;

            aul::uninitialized_copy(segment0.first, segment0.second, allocation.array, allocator);
            aul::uninitialized_copy(segment1.first, segment1.second, allocation.array + segment0_size, allocator);

            return *this;
        }

        /// Move assignment operator
        /// \param rhs Object to move resources from
        /// \return *this
        constexpr Circular_array& operator=(Circular_array&& rhs) noexcept(aul::is_noexcept_movable<A>::value) {
            clear();

            //TODO: Provide strong exception gaurentee
            if constexpr (std::allocator_traits<A>::propagate_on_container_move_assignment::value) {
                allocator = std::move(allocator);
                allocation = std::move(rhs.allocation);
                head_offset = rhs.head_offset;
            } else {
                allocation = allocate(rhs.size());
                head_offset = 0;
                aul::uninitialized_move(rhs.begin(), rhs.end(), allocation.array, allocator);
                rhs.deallocate(rhs.allocation);
            }
            elem_count = rhs.elem_count;

            rhs.elem_count = 0;
            rhs.head_offset = 0;

            return *this;
        }

        ///
        /// \param list List to copy elements from
        /// \return *this
        constexpr Circular_array& operator=(std::initializer_list<T> list) {
            //TODO: Provide strong exception gaurentee
            clear();

            elem_count = list.end() - list.begin();
            allocation = allocate(elem_count);
            head_offset = 0;

            aul::uninitialized_copy(list.begin(), list.end(), allocation.array, allocator);

            return *this;
        }

        ///
        /// \tparam Iter Forward iterator type
        /// \param from Iterator to beginning of range
        /// \param to   Iterator to end of range
        template<class Iter>
        void assign(Iter from, Iter to) {
            //TODO: Provide strong exception guarantee
            clear();

            elem_count = to - from;
            allocation = allocate(elem_count);
            head_offset = 0;

            aul::uninitialized_copy(from, to, allocation.array, allocator);
        }

        void assign(const std::initializer_list<T>& list) {
            assign(list.begin(), list.end());
        }

        void assign(const size_type n, const T& val) {
            //TODO: Provide strong exception guarantee

            clear();

            elem_count = n;
            allocation = allocate(n);
            head_offset = 0;

            aul::uninitialized_fill(allocation.array, allocation.array + n, val, allocator);
        }

        //=================================================
        // Iterator methods
        //=================================================

        ///
        /// \return
        constexpr iterator begin() {
            if (empty()) {
                return iterator{};
            } else {
                return iterator{
                    static_cast<difference_type>(head_offset - capacity()),
                    allocation.array,
                    allocation.array + capacity()
                };
            }
        }

        constexpr const_iterator begin() const {
            if (empty()) {
                return const_iterator{};
            } else {
                return const_iterator {
                    static_cast<difference_type>(head_offset - capacity())  ,
                    allocation.array,
                    allocation.array + capacity()
                };
            }
        }

        constexpr const_iterator cbegin() const {
            return const_cast<const Circular_array&>(*this).begin();
        }

        constexpr iterator end() {
            if (empty()) {
                return iterator{};
            } else {
                return iterator{
                    static_cast<difference_type>(head_offset - capacity() + size()),
                    allocation.array,
                    allocation.array + capacity()
                };
            }
        }

        constexpr const_iterator end() const {
            if (empty()) {
                return const_iterator{};
            }
            else {
                return const_iterator{
                    static_cast<difference_type>(head_offset - capacity() + size()),
                    allocation.array,
                    allocation.array + capacity()
                };
            }
        }

        constexpr const_iterator cend() const {
            return const_cast<const Circular_array&>(*this).end();
        }



        constexpr reverse_iterator rbegin() {
            return reverse_iterator{end()};
        }

        constexpr const_reverse_iterator rbegin() const {
            return const_reverse_iterator{end()};
        }

        constexpr const_reverse_iterator crbegin() const {
            return const_cast<const Circular_array&>(*this).rbegin();
        }

        constexpr reverse_iterator rend() {
            return reverse_iterator{begin()};
        }

        constexpr const_reverse_iterator rend() const {
            return const_reverse_iterator{begin()};
        }

        constexpr const_reverse_iterator crend() const {
            return const_cast<const Circular_array&>(*this).rend();
        }

        //=================================================
        // Element accessors
        //=================================================

        constexpr T& front() {
            return allocation.array[head_offset];
        }

        constexpr const T& front() const {
            return allocation.array[head_offset];
        }

        constexpr T& back() {
            return *index_to_ptr(elem_count);
        }

        constexpr const T& back() const {
            return *index_to_ptr(elem_count);
        }

        constexpr T& operator[](const size_type i) {
            return *index_to_ptr(i);
        }

        constexpr const T& operator[](const size_type i) const {
            return *index_to_ptr(i);
        }

        constexpr T& at(const size_type i) {
            if (size() <= i) {
                throw std::out_of_range("Circular_array::at() called with invalid index");
            }
            return *index_to_ptr(i);
        }

        constexpr const T& at(const size_type i) const {
            if (size() <= i) {
                throw std::out_of_range("Circular_array::at() called with invalid index");
            }
            return *index_to_ptr(i);
        }

        //=================================================
        // Element addition
        //=================================================

        ///
        /// Provides the strong exception guarantee.
        ///
        /// \tparam Args
        /// \param pos
        /// \param args
        template<class...Args>
        iterator emplace(const_iterator p, Args...args) {
            iterator pos = iterator{p.offset, const_cast<pointer>(p.begin), const_cast<pointer>(p.end)};
            size_type left_count = pos - begin();
            size_type right_count = cend() - pos;

            if (size() < capacity()) {

                //Move other elements out of the way
                if (left_count < right_count) {
                    if (left_count) {
                        std::allocator_traits<A>::construct(allocator, (begin() - 1).operator->(), std::move(*begin()));
                        for (auto it = begin(); it++ != pos;) {
                            it[0] = std::move(it[1]);
                        }
                    }
                } else {
                    if (right_count) {
                        std::allocator_traits<A>::construct(allocator, end().operator->(), std::move(end()[-1]));
                        for (auto it = end() - 1; it-- != pos;) {
                            it[-1] = std::move(it[0]);
                        }
                    }
                }

                //Construct new element
                auto ptr = pos.operator->();
                std::allocator_traits<A>::destroy(allocator, ptr);
                try {
                    std::allocator_traits<A>::construct(allocator, ptr, std::forward<Args>(args)...);
                } catch(...) {

                    //Move elements back if failed to construct
                    if (left_count < right_count) {
                        if (left_count) {
                            std::allocator_traits<A>::construct(allocator, ptr, std::move(pos[-1]));
                            const iterator beg = begin();
                            for (iterator it = pos - 1; it-- != beg;) {
                                it[0] = std::move(it[-1]);
                            }
                        }
                    } else {
                        if (right_count) {
                            std::allocator_traits<A>::construct(allocator, ptr, std::move(pos[1]));
                            const iterator e = end();
                            for (auto it = pos + 1; pos++ != e;) {
                                it[0] = std::move(it[0]);
                            }
                        }
                    }

                    throw;
                }

            } else {
                Allocation new_allocation = allocate(grow_size(size() + 1));

                //Construct new element
                if (empty() && begin() == pos) {
                    std::allocator_traits<A>::construct(allocator, new_allocation.array, std::forward<Args>(args)...);
                } else {
                    pointer ptr = pos.operator->();
                    std::allocator_traits<A>::construct(allocator, ptr, std::forward<Args>(args)...);

                    //Copy left elements
                    aul::uninitialized_move(begin(), pos, allocation.array,allocator);

                    //Copy right elements
                    aul::uninitialized_move(pos + 1, end(), ptr + 1, allocator);
                }

                //Free old allocation and replace
                deallocate(allocation);
                allocation = std::move(new_allocation);
            }

            elem_count++;
            return pos;
        }

        iterator insert(const_iterator pos, const T& val) {
            return emplace(pos, val);
        }

        iterator insert(const_iterator pos, T&& val) {
            return emplace(pos, val);
        }

        iterator insert(const_iterator pos, const size_type n, const T& val);

        template<class Iter>
        iterator insert(const_iterator pos, Iter from, Iter to);

        iterator insert(const_iterator pos, const std::initializer_list<value_type>& list);

        template<class...Args>
        void emplace_front(Args...args) {
            if (size() + 1 <= capacity()) {
                pointer ptr = (begin() - 1).operator->();
                std::allocator_traits<A>::construct(allocator, ptr, std::forward<Args>(args)...);

                head_offset = ptr - allocation.array;
            } else {
                size_type new_capacity = grow_size(size() + 1);
                Allocation new_allocation = allocate(new_capacity);

                difference_type new_offset = new_allocation.capacity - size() - 1;
                pointer ptr = new_allocation.array + new_offset;

                std::allocator_traits<A>::construct(allocator, ptr, std::forward<Args>(args)...);

                aul::uninitialized_move(begin(), end(), ptr + 1, allocator);

                head_offset = new_offset;

                deallocate(allocation);
                allocation = std::move(new_allocation);
            }
            elem_count++;
        }

        void push_front(const T& val) {
            emplace_front(val);
        }

        void push_front(T&& val) {
            emplace_front(std::forward<T&&>(val));
        }

        template<class...Args>
        void emplace_back(Args...args) {
            if (size() + 1 <= capacity()) {
                pointer ptr = end().operator->();

                std::allocator_traits<A>::construct(allocator, ptr, std::forward<Args>(args)...);
            } else {
                size_type new_capacity = grow_size(size() + 1);
                Allocation new_allocation = allocate(new_capacity);

                std::allocator_traits<A>::construct(allocator, new_allocation.array + size(), std::forward<Args>(args)...);

                auto x = begin();
                auto y = end();
                aul::uninitialized_move(begin(), end(), new_allocation.array, allocator);

                head_offset = 0;

                deallocate(allocation);
                allocation = std::move(new_allocation);
            }
            elem_count++;
        }

        void push_back(const T& val) {
            emplace_back(val);
        }

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
        void pop_front() {
            pointer ptr = begin().operator->();
            std::allocator_traits<A>::destroy(allocator, ptr);

            ++head_offset;
            if (head_offset == capacity()) {
                head_offset = 0;
            }

            --elem_count;
        }

        /// 
        /// Removes the last element in the container. Undefined behavior if
        /// container is empty.
        ///
        void pop_back() {
            pointer ptr = (end() - 1).operator->();
            std::allocator_traits<A>::destroy(allocator, ptr);
            --elem_count;
        }


        /// 
        /// Undefined behavior if pos does not point to an element
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
                std::allocator_traits<allocator_type>::destroy(allocator, begin().operator->());
            } else {
                for (auto it = end() - 1; it++ != it;) {
                    it[0] = it[1];
                }
                std::allocator_traits<allocator_type>::destroy(allocator, (end() - 1).operator->());
            }
            --elem_count;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="from"></param>
        /// <param name="to"></param>
        void erase(const_iterator from, const_iterator to);

        //=================================================
        // State Mutators
        //=================================================

        void reserve(const size_type n) {
            if (n <= size()) {
                return;
            }

            if (max_size() < n) {
                throw std::length_error("aul::Circular_array::reserve() called with excessive allocation size");
            }

            Allocation new_allocation = allocate(n, allocation);

            if (new_allocation.array == allocation.array) {
                //Allocation was extended

                if(is_segmented()) {
                    auto [l, r] = second_segment();
                    aul::uninitialized_move(l,r, new_allocation.array + new_allocation.capacity - (r - l), allocator);
                    for (;l != r; ++l) {
                        std::allocator_traits<allocator_type>::destroy(allocator, l);
                    }
                }
            } else {
                //Completely new allocation
                auto [l0, r0] = first_segment();
                auto [l1, r1] = second_segment();

                pointer dest = new_allocation.array + capacity() / 2 - size() / 2;
                pointer end = dest + size();

                aul::uninitialized_move(l0, r0, dest, allocator);
                aul::uninitialized_move(l1, r1, dest + (r0 - l0), allocator);
            }

            allocation = std::move(new_allocation);
        }

        //=================================================
        // State accessors
        //=================================================

        [[nodiscard]]
        constexpr bool empty() const {
            return elem_count == 0;
        }

        [[nodiscard]]
        constexpr size_type size() const {
            return elem_count;
        }

        [[nodiscard]]
        size_type max_size() const {
            constexpr auto type_max = static_cast<size_type>(
                std::numeric_limits<difference_type>::max()
            );

            const size_type max_allocation = std::allocator_traits<A>::max_size(allocator);

            return std::min(max_allocation, type_max);
        }

        [[nodiscard]]
        constexpr size_type capacity() const {
            return allocation.capacity;
        }

        [[nodiscard]]
        constexpr allocator_type get_allocator() const {
            return allocator;
        }

        [[nodiscard]]
        std::tuple<pointer, pointer, pointer, pointer> data() {
            const auto segment0 = first_segment();
            const auto segment1 = second_segment();
            return {segment0.first, segment0.second, segment1.first, segment1.second};
        }

        [[nodiscard]]
        std::tuple<const_pointer, const_pointer, const_pointer, const_pointer> data() const {
            const auto segment0 = first_segment();
            const auto segment1 = second_segment();
            return {segment0.first, segment0.second, segment1.first, segment1.second};
        }

        //=================================================
        // Misc. methods
        //=================================================

        ///
        /// Destroys all elements and frees currently held allocation
        ///
        constexpr void clear() {
            if (!empty()) {
                clear_first_segment();
                clear_second_segment();
            }

            if (capacity() != 0) {
                deallocate(allocation);
            }
            elem_count = 0;
            head_offset = 0;
        }

    private:

        //=================================================
        // Instance members
        //=================================================

        Allocation allocation{};

        allocator_type allocator{};

        difference_type head_offset{};

        size_type elem_count{};

        //=================================================
        // Helper functions
        //=================================================

        [[nodiscard]]
        constexpr pointer index_to_ptr(const size_type n) const {
            if (head_offset + size() <= capacity()) {
                return allocation.array + n;
            } else {
                return allocation.array + size() - capacity() + head_offset;
            }
        }

        ///
        /// \return True if elements wrap around after reaching end of
        /// allocation.
        ///
        [[nodiscard]]
        constexpr bool is_segmented() const {
            return static_cast<difference_type>(size()) > (capacity() - head_offset);
        }

        [[nodiscard]]
        constexpr std::pair<pointer, pointer> first_segment() {
            if (is_segmented()) {
                return {
                    allocation.array + head_offset,
                    allocation.array + capacity()
                };
            } else {
                return {
                    allocation.array + head_offset,
                    allocation.array + head_offset + size()
                };
            };
        }

        [[nodiscard]]
        constexpr std::pair<const_pointer, const_pointer> first_segment() const {
            if (is_segmented()) {
                return {
                    allocation.array + head_offset,
                    allocation.array + capacity()
                };
            }
            else {
                return {
                    allocation.array + head_offset,
                    allocation.array + head_offset + size()
                };
            };
        }

        [[nodiscard]]
        constexpr std::pair<pointer, pointer> second_segment() {
            if (is_segmented()) {
                return {
                    allocation.array,
                    allocation.array + head_offset - capacity() + size()
                };
            } else {
                return {pointer{}, pointer{}};
            }
        }

        [[nodiscard]]
        constexpr std::pair<const_pointer, const_pointer> second_segment() const {
            if (is_segmented()) {
                return {
                    allocation.array,
                    allocation.array + head_offset - capacity() + size()
                };
            }
            else {
                return { pointer{}, pointer{} };
            }
        }

        [[nodiscard]]
        constexpr Allocation allocate(const size_type n) {
            Allocation alloc{};

            try {
                alloc.array = std::allocator_traits<allocator_type>::allocate(allocator, n);
                alloc.capacity = n;
            } catch (...) {
                alloc = {};
                throw;
            }

            return alloc;
        }

        [[nodiscard]]
        constexpr Allocation allocate(const size_type n, const Allocation& hint) {
            Allocation alloc{};

            try {
                alloc.array = std::allocator_traits<allocator_type>::allocate(allocator, n, hint.array);
                alloc.capacity = n;
            } catch (...) {
                alloc = {};
                throw;
            }

            return alloc;
        }

        constexpr void deallocate(Allocation& alloc) {
            std::allocator_traits<allocator_type>::deallocate(allocator, alloc.array, alloc.capacity);
            alloc = {};
        }

        constexpr void clear_first_segment() {
            auto [l, r] = first_segment();
            for (;l != r; ++l) {
                std::allocator_traits<allocator_type>::destroy(allocator, l);
            }
        }

        constexpr void clear_second_segment() {
            auto [l, r] = second_segment();
            for (;l != r; ++l) {
                std::allocator_traits<allocator_type>::destroy(allocator, l);
            }
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

    };



    template<class T, class A>
    template<bool is_const>
    class Circular_array<T, A>::Iterator {
    public:
        friend class aul::Circular_array<T, A>;

        //=================================================
        // Type aliases
        //=================================================

        using value_type = typename Circular_array<T, A>::value_type;
        using difference_type = typename Circular_array<T, A>::difference_type;

        using reference  = std::conditional_t<is_const,
            typename Circular_array<T, A>::const_reference,
            typename Circular_array<T, A>::reference
        >;

        using pointer  = std::conditional_t<is_const,
            typename Circular_array<T, A>::const_pointer,
            typename Circular_array<T, A>::pointer
        >;

        using iterator_category = std::random_access_iterator_tag;

        //=================================================
        // -ctors
        //=================================================

        constexpr Iterator() = default;

        constexpr Iterator(const difference_type offset, pointer a, pointer b):
            offset(offset),
            begin(a),
            end(b) {}

        constexpr Iterator(const Iterator& it) = default;

        constexpr Iterator(Iterator&& it) noexcept:
            offset(it.offset),
            begin(it.begin),
            end(it.end) {

            it.offset = 0;
            it.begin = nullptr;
            it.end = nullptr;
        }

        ~Iterator() = default;

        //=================================================
        // Assignment operators/methods
        //=================================================

        [[nodiscard]]
        constexpr Iterator operator=(const Iterator& it) {
            offset = it.offset;
            begin = it.begin;
            end = it.end;

            return *this;
        }

        [[nodiscard]]
        constexpr Iterator operator=(Iterator&& it) {
            offset = it.offset;
            begin = it.begin;
            end = it.end;

            it = {};

            return *this;
        }

        //=================================================
        // Comparison operators
        //=================================================

        [[nodiscard]]
        constexpr bool operator==(const Iterator it) const {
            return (offset == it.offset) && (begin == it.begin) && (end == it.end);
        }

        [[nodiscard]]
        constexpr bool operator!=(const Iterator it) const {
            return (offset != it.offset) || (begin != it.begin) || (end != it.end);
        }

        [[nodiscard]]
        constexpr bool operator<(const Iterator it) const {
            return offset < it.offset;
        }

        [[nodiscard]]
        constexpr bool operator>(const Iterator it) const {
            return offset > it.offset;
        }

        [[nodiscard]]
        constexpr bool operator<=(const Iterator it) const {
            return offset <= it.offset;
        }

        [[nodiscard]]
        constexpr bool operator>=(const Iterator it) const {
            return offset >= it.offset;
        }

        //=================================================
        // Increment/Decrement operators
        //=================================================

        constexpr Iterator operator++() {
            ++offset;
            return *this;
        }

        constexpr Iterator operator++(int) {
            auto temp = *this;
            ++offset;
            return temp;
        }

        constexpr Iterator operator--() {
            --offset;
            return *this;
        }

        constexpr Iterator operator--(int) {
            auto temp = *this;
            --offset;
            return temp;
        }

        //=================================================
        // Arithmetic operators
        //=================================================

        [[nodiscard]]
        constexpr Iterator operator+(const difference_type x) const {
            auto temp = *this;
            temp.offset += x;
            return temp;
        }

        [[nodiscard]]
        constexpr Iterator operator-(const difference_type x) const {
             auto temp = *this;
             temp.offset -= x;
             return temp;
        }

        [[nodiscard]]
        friend constexpr Iterator operator+(const difference_type x, const Iterator it) {
            it.offset += x;
            return it;
        }

        [[nodiscard]]
        constexpr difference_type operator-(const Iterator it) const {
            return offset - it.offset;
        }

        //=================================================
        // Arithmetic assignment operators
        //=================================================

        [[nodiscard]]
        constexpr Iterator operator+=(const difference_type x) {
            offset += x;
            return *this;
        }

        [[nodiscard]]
        constexpr Iterator operator-=(const difference_type x) {
            offset -= x;
            return *this;
        }

        //=================================================
        // Dereference operators
        //=================================================

        [[nodiscard]]
        constexpr reference operator*() const {
            return *operator->();
        }

        [[nodiscard]]
        constexpr reference operator[](const difference_type x) const {
            return *(*this + x);
        }

        [[nodiscard]]
        constexpr pointer operator->() const {
            return (offset < 0 ? end  : begin) + offset;
        }

        //=================================================
        // Conversion operators
        //=================================================

        [[nodiscard]]
        operator Iterator<true>() const {
            return {offset, begin, end};
        }

    private:
        difference_type offset{};
        pointer begin{};
        pointer end{};
    };



    template<class T, class A>
    class Circular_array<T, A>::Allocation {
    public:

        //=================================================
        // -ctors
        //=================================================

        constexpr Allocation() = default;

        Allocation(const Allocation&) = delete;

        constexpr Allocation(Allocation&& alloc) noexcept:
            array(alloc.array),
            capacity(alloc.capacity) {

            alloc = {};
        }

        ~Allocation() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Allocation& operator=(const Allocation&) = delete;

        constexpr Allocation& operator=(Allocation&& rhs) noexcept {
            array = rhs.array;
            capacity = rhs.capacity;

            rhs.array = nullptr;
            rhs.capacity = 0;

            return {*this};
        }

        //=================================================
        // Instance members
        //=================================================

        pointer array{};
        size_type capacity{};

    };

}

#endif //AUL_CIRCULAR_ARRAY_HPP

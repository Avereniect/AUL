//
// Created by avereniect on 5/10/20.
//

#ifndef AUL_CIRCULAR_ARRAY_HPP
#define AUL_CIRCULAR_ARRAY_HPP

#include "Algorithms.hpp"
#include "memory/Memory.hpp"

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

        using pointer = typename std::allocator_traits<allocator_type>::pointer;
        using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;

        using iterator = Iterator<false>;
        using const_iterator = Iterator<true>;

        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        //=================================================
        // -ctors
        //=================================================

        constexpr Circular_array() = default;

        constexpr Circular_array(const Circular_array& arr);
        constexpr Circular_array(const Circular_array&, const allocator_type& alloc);

        constexpr Circular_array(Circular_array&&);
        constexpr Circular_array(Circular_array&&, const allocator_type& alloc);

        constexpr Circular_array(const size_type n):
            allocator({}),
            allocation(allocate(n)),
            elem_count(n) {

            aul::default_construct(allocation.array, allocation.array + n, allocator);
        }

        constexpr Circular_array(const size_type n, const allocator_type& alloc):
            allocator(alloc),
            allocation(allocate(n)),
            elem_count(n) {

            aul::default_construct(allocation.array, allocation.array + n, allocator);
        }

        constexpr Circular_array(const size_type n, const T& val):
            allocator({}),
            allocation(allocate(n)),
            elem_count(n) {

            std::uninitialized_fill(allocation.array, allocation.array + n, val, allocator);
        }

        constexpr Circular_array(const size_type n, const T& val, const allocator_type& alloc):
            allocator(alloc),
            allocation(allocate(n)),
            elem_count(n) {

            std::uninitialized_fill(allocation.array, allocation.array + n, val, allocator);
        }

        constexpr Circular_array(const std::initializer_list<T>& list):
            allocator({}),
            allocation(allocate(list.end() - list.begin())),
            elem_count(list.end() - list.begin()) {

            aul::uninitialized_copy(allocation.array, allocation.array + size(), allocator);
        }

        constexpr Circular_array(const std::initializer_list<T>& list, const allocator_type& alloc):
            allocator(alloc),
            allocation(allocate(list.end() - list.begin())),
            elem_count(list.end() - list.begin()) {

            aul::uninitialized_copy(allocation.array, allocation.array + size(), allocator);
        }

        ~Circular_array() {
            clear();
        }

        //=================================================
        // Assignment operators
        //=================================================

        Circular_array& operator=(const Circular_array& rhs) {
            clear();
            allocation = allocate(rhs.size());
            head_offset = 0;
            elem_count = rhs.elem_count;

            const auto segment0 = rhs.first_segment();
            const auto segment1 = rhs.second_segment();

            const auto segment0_size = segment0.second - segment0.first;

            aul::uninitialized_copy(segment0.first, segment0.second, allocation.array, allocator);
            aul::uninitialized_copy(segment1.first, segment1.second, allocation.array + segment0_size, allocator);
        }

        constexpr Circular_array& operator=(Circular_array&& rhs) noexcept {
            clear();

            allocator = std::move(allocator);
            allocation = std::move(rhs.allocation);
            head_offset = rhs.head_offset;
            elem_count = rhs.elem_count;

            rhs.elem_count = 0;
            rhs.head_offset = 0;

            return *this;
        }

        //=================================================
        // Iterator methods
        //=================================================

        iterator begin() {
            if (empty()) {
                return iterator{};
            } else {
                return iterator {
                    head_offset - capacity(),
                    allocation.array,
                    allocation.array + capacity()
                };
            }
        }

        const_iterator begin() const {
            if (empty()) {
                return const_iterator{};
            } else {
                return const_iterator {
                    head_offset - capacity(),
                    allocation.array,
                    allocation.array + capacity()
                };
            }
        }

        const_iterator cbegin() const {
            return const_cast<const Circular_array&>(*this).begin();
        }

        iterator end() {
            if (empty()) {
                return iterator{};
            } else {
                return iterator{
                    head_offset - capacity() + size(),
                    allocation.array,
                    allocation.array + capacity()
                };
            }
        }

        //=================================================
        // Element accessors
        //=================================================

        T& front() {
            return allocation.array[head_offset];
        }

        const T& front() const {
            return allocation.array[head_offset];
        }

        T& back() {
            return *index_to_ptr(elem_count);
        }

        const T& back() const {
            return *index_to_ptr(elem_count);
        }

        T& operator[](const size_type i) {
            return *index_to_ptr(i);
        }

        const T& operator[](const size_type i) const {
            return *index_to_ptr(i);
        }

        T& at(const size_type i) {
            if (size() < i) {
                throw std::out_of_range("Circular_array::at() called with invalid index");
            }
            return *index_to_ptr(i);
        }

        const T& at(const size_type i) const {
            if (size() < i) {
                throw std::out_of_range("Circular_array::at() called with invalid index");
            }
            return *index_to_ptr(i);
        }

        //=================================================
        // Element addition
        //=================================================

        template<class...Args>
        void emplace_front(Args...args) {
            if (max_size() < elem_count + 1) {
                throw std::length_error("aul::Circular_array grew too big");
            }

            if (capacity() <= size() + 1) {
                Allocation allocation;

            } else {
                //Move elements
                if (is_segmented() &&) {

                } else {

                }
                //Construct new element

            }

            ++elem_count;
        }

        void push_front(const T& val) {
            emplace_front(val);
        }

        void push_front(T&& val) {
            emplace_front(std::forward(val));
        }

        template<class...Args>
        void emplace_back(Args...args);

        void push_back(const T& val) {
            emplace_back(val);
        }

        void push_back(T&& val) {
            emplace_back(std::forward(val));
        }

        template<class...Args>
        void emplace(const_iterator it, Args...args);

        void insert(const_iterator it, const T& val) {
            emplace(it, val);
        }

        void insert(const_iterator it, T&& val) {
            emplace(it, std::forward<T&&>(val));
        }

        //=================================================
        // Element removal
        //=================================================

        void pop_front() {
            pointer ptr = begin().operator->();
            std::allocator_traits<allocator_type>::destroy(ptr);

            ++head_offset;
            if (head_offset == capacity()) {
                head_offset = 0;
            }

            --elem_count;
        }

        void pop_back() {
            pointer ptr = (end() - 1).operator->();
            std::allocator_traits<allocator_type>::destroy(ptr);
            --elem_count;
        }

        void erase(const_iterator pos) {
            size_type left = pos - begin();
            size_type right = end() - pos - 1;

            if (left < right) {
                for (auto  = begin(); it != pos; ++it) {
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

        //=================================================
        // State Mutators
        //=================================================

        void reserve(const size_type n) {
            if (n <= size()) {
                return;
            }

            if (max_size() < n) {reserve(
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
                auto [l1, r1] = second_sement();

                pointer dest = allocation / 2 - size / 2;
                pointer end = dest + size();

                aul::uninitialized_move(l0, r0, dest, allocator);
                aul::uninitialized_move(l1, r1, dest + (r0 - l0), allocator);
            }
        }

        //=================================================
        // State accessors
        //=================================================

        [[nodiscard]]
        bool empty() const {
            return elem_count == 0;
        }

        [[nodiscard]]
        size_type size() const {
            return elem_count;
        }

        [[nodiscard]]
        size_type max_size() const {
            constexpr auto type_max = static_cast<size_type>(
                std::numeric_limits<difference_type>::max()
            );

            const size_type max_allocation = allocator.max_size();

            return std::min(max_allocation, type_max);
        }

        [[nodiscard]]
        size_type capacity() const {
            return allocation.capacity;
        }

        [[nodiscard]]
        allocator_type get_allocator() const {
            return allocator;
        }

        [[nodiscard]]
        std::tuple<pointer, size_type, pointer, size_type> data() {
            const auto segment0 = first_segment();
            const auto segment1 = second_segment();
            return {segment0.first, segment0.second, segment1.first, segment1.second};
        }

        [[nodiscard]]
        std::tuple<const_pointer, size_type, const_pointer, size_type> data() const {
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
        void clear() {
            clear_first_segment();
            clear_second_segment();

            deallocate(allocation);
        }

    private:

        //=================================================
        // Instance members
        //=================================================

        Allocation allocation{};

        allocator_type allocator{};

        size_type head_offset{};

        size_type elem_count{};

        //=================================================
        // Helper functions
        //=================================================

        [[nodiscard]]
        pointer index_to_ptr(const size_type n) const {
            if (head_offset + size() < capacity()) {
                return allocation.array + n;
            } else {
                return allocation.array + size() - capacity() + head_offset();
            }
        }

        ///
        /// \return True if elements wrap around after reaching end of
        /// allocation.
        ///
        [[nodiscard]]
        bool is_segmented() const {
            return static_cast<difference_type>(size()) > (capacity() - head_offset);
        }

        [[nodiscard]]
        std::pair<pointer, pointer> first_segment() const {
            if (is_segmented()) {
                return {
                    allocation.array + head_offset,
                    allocation.array + capacity()
                };
            } else {
                return {
                    allocation.array + head_offset,
                    head_offset + size()
                };
            };
        }

        [[nodiscard]]
        std::pair<pointer, pointer> second_segment() {
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
        Allocation allocate(const size_type n) {
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
        Allocation allocate(const size_type n, const Allocation& hint) {
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

        void deallocate(Allocation& alloc) {
            std::allocator_traits<allocator_type>::deallocate(allocator, alloc.array);
            alloc.clear();
        }

        void clear_first_segment() {
            auto [l, r] = first_segment();
            for (;l != r; ++l) {
                std::allocator_traits<allocator_type>::destroy(allocator, l);
            }
        }

        void clear_second_segment() {
            auto [l, r] = second_segment();
            for (;l != r; ++l) {
                std::allocator_traits<allocator_type>::destroy(allocator, l);
            }
        }
    };



    template<class T, class A>
    template<bool is_const>
    class Circular_array<T, A>::Iterator {
    public:

        //=================================================
        // Type aliases
        //=================================================

        using value_type = typename Circular_array<T, A>::value_type;
        using differnce_type = typename Circular_array<T, A>::difference_type;

        using reference  = std::conditional<is_const,
            typename Circular_array<T, A>::const_reference,
            typename Circular_array<T, A>::reference
        >;

        using pointer  = std::conditional<is_const,
            typename Circular_array<T, A>::const_pointer,
            typename Circular_array<T, A>::pointer
        >;

        using iterator_tag = std::random_access_iterator_tag;

        //=================================================
        // -ctors
        //=================================================

        constexpr Iterator() = default;

        constexpr Iterator(const difference_type offset, const pointer a, const pointer b):
            offset(offset),
            begin(a),
            end(b) {}

        constexpr Iterator(const Iterator& it) = default;

        constexpr Iterator(Iterator&& it):
            offset(it.offset),
            begin(it.begin),
            end(it.end) {

            it = {};
        }

        ~Iterator() = default;

        //=================================================
        // Assignment operators
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
        constexpr bool operator==(const Iterator it) {
            return (offset == it.offset) && (begin == it.begin) && (end == it.end);
        }

        [[nodiscard]]
        constexpr bool operator!=(const Iterator it) {
            return (offset != it.offset) || (begin != it.begin) || (end != it.end);
        }

        [[nodiscard]]
        constexpr bool operator<(const Iterator it) {
            return offset < it.offset;
        }

        [[nodiscard]]
        constexpr bool operator>(const Iterator it) {
            return offset > it.offset;
        }

        [[nodiscard]]
        constexpr bool operator<=(const Iterator it) {
            return offset <= it.offset;
        }

        [[nodiscard]]
        constexpr bool operator>=(const Iterator it) {
            return offset >= it.offset;
        }

        //=================================================
        // Increment/Decrement operators
        //=================================================

        [[nodiscard]]
        constexpr Iterator operator++() {
            ++offset;
            return;
        }

        [[nodiscard]]
        constexpr Iterator operator++(int) {
            auto temp = *this;
            ++offset;
            return temp;
        }

        [[nodiscard]]
        constexpr Iterator operator--() {
            --offset;
            return *this;
        }

        [[nodiscard]]
        constexpr Iterator operator--(int) {
            auto temp = *this;
            --offset;
            return temp;
        }

        //=================================================
        // Arithmetic operators
        //=================================================

        [[nodiscard]]
        constexpr Iterator operator+(const difference_type x) {
            auto temp = *this;
            temp.offset += x;
            return temp;
        }

        [[nodiscard]]
        constexpr Iterator operator-(const difference_type x) {
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
        constexpr Iterator operator-(const Iterator it) {
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
        constexpr reference operator*() {
            return *operator->();
        }

        [[nodiscard]]
        constexpr reference operator[](const difference_type x) {
            return *(*this + x);
        }

        [[nodiscard]]
        constexpr pointer operator->() {
            return (0 < offset) ? begin + offset : end - offset;
        }

        //=================================================
        // Conversion operators
        //=================================================

        [[nodiscard]]
        operator Iterator<true>() {
            return {offset, begin, end};
        }

    private:
        difference_type offset{};
        Circular_array::pointer begin{};
        Circular_array::pointer end{};
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

        constexpr Allocation& operator=(Allocation&& rhs) {
            array = rhs.array;
            capacity = rhs.array;

            rhs = {};

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

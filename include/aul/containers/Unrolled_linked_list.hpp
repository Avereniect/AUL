#ifndef AUL_UNROLLED_LINKED_LIST_HPP
#define AUL_UNROLLED_LINKED_LIST_HPP

#include "Allocator_aware_base.hpp"

#include <cstdint>
#include <limits>
#include <memory>
#include <initializer_list>

namespace aul {

    template<class P, std::uint32_t N>
    class Unrolled_linked_list_node {
    public:

        //=================================================
        // Type aliases
        //=================================================

        using element_type = typename std::pointer_traits<P>::element_type;
        using reference = element_type&;
        using const_reference = const element_type&;

        using difference_type = typename std::pointer_traits<P>::difference_type;

        using pointer = P;
        using const_pointer = typename std::pointer_traits<P>::template rebind<const element_type>;
        using node_pointer = typename std::pointer_traits<P>::template rebind<Unrolled_linked_list_node>;

        //=================================================
        // Static members
        //=================================================

        static constexpr std::uint32_t capacity = N;

        //=================================================
        // Accessors
        //=================================================

        [[nodiscard]]
        reference operator[](difference_type i) {
            return data[i];
        }

        [[nodiscard]]
        const_reference operator[](difference_type i) const {
            return data[i];
        }

        //=================================================
        // Instance members
        //=================================================

        char data[N * sizeof(element_type)];

        node_pointer prev = nullptr;
        node_pointer next = nullptr;

        difference_type elem_count = 0;
    };





    template<class P, std::uint32_t N>
    class Unrolled_linked_list_iterator {
    public:

        //=================================================
        // Type aliases
        //=================================================

        using value_type = typename std::pointer_traits<P>::element_type::value_type;
        using difference_type = typename std::pointer_traits<P>::difference_type;
        using pointer = P;
        using reference = value_type&;
        using iterator_category = std::bidirectional_iterator_tag;

        //=================================================
        // -ctors
        //=================================================

        Unrolled_linked_list_iterator(P ptr, std::uint32_t offset):
            ptr(ptr),
            offset(offset) {}

        ~Unrolled_linked_list_iterator() = default;

        //=================================================
        // Assignment operators
        //=================================================

        //=================================================
        // Comparison operators
        //=================================================

        friend bool operator==(Unrolled_linked_list_iterator lhs, Unrolled_linked_list_iterator rhs) {
            return lhs.ptr == rhs.ptr && lhs.offset == rhs.offset;
        }

        friend bool operator!=(Unrolled_linked_list_iterator lhs, Unrolled_linked_list_iterator rhs) {
            return lhs.ptr != rhs.ptr && lhs.offset != rhs.offset;
        }

        //=================================================
        // Increment/decrement operators
        //=================================================

        Unrolled_linked_list_iterator& operator++() {
            offset += 1;
            if (offset == N) {
                offset = 0;
                ptr = ptr->next;
            }
            return *this;
        }

        Unrolled_linked_list_iterator operator++(int) {
            auto ret = *this;
            operator++();
            return ret;
        }

        Unrolled_linked_list_iterator& operator--() {
            if (offset == 0) {
                offset = N - 1;
                ptr = ptr->prev;
            } else {
                offset -= 1;
            }
            return *this;
        }

        Unrolled_linked_list_iterator operator--(int) {
            auto ret = *this;
            operator--();
            return ret;
        }

        //=================================================
        // Dereference operators
        //=================================================

        [[nodiscard]]
        pointer operator->() const {
            return ptr.data() + offset;
        }

        [[nodiscard]]
        reference operator*() const {
            return ptr[offset];
        }

    private:

        //=================================================
        // Instance members
        //=================================================

        pointer ptr;
        std::uint32_t offset;

    };





    template<class T, std::size_t N = 4 * 64 / sizeof(T), class A = std::allocator<T>>
    class Unrolled_linked_list :
        public aul::Allocator_aware_base<
            typename std::allocator_traits<A>::template rebind_alloc<
                Unrolled_linked_list_node<
                    typename std::allocator_traits<A>::pointer,
                    N
                >
            >
        > {

        using base = aul::Allocator_aware_base<
            typename std::allocator_traits<A>::template rebind_alloc<
                Unrolled_linked_list_node<
                    typename std::allocator_traits<A>::pointer,
                    N
                >
            >
        >;

    public:

        //=================================================
        // Type aliases
        //=================================================

        using value_type = T;

        using pointer = typename std::allocator_traits<A>::pointer;
        using const_pointer = typename std::allocator_traits<A>::const_pointer;

        using reference = T&;
        using const_reference = const T&;

        using size_type = typename std::allocator_traits<A>::size_type;
        using difference_type = typename std::allocator_traits<A>::difference_type;

        using allocator = A;

        using iterator = Unrolled_linked_list_iterator<pointer, N>;
        using const_iterator = Unrolled_linked_list_iterator<const_pointer, N>;

    private:

        using node = Unrolled_linked_list_node<pointer, N>;

        using node_allocator = typename std::allocator_traits<A>::template rebind_alloc<node>;

        using node_pointer = typename std::allocator_traits<node_allocator>::pointer;

    public:

        //=================================================
        // -ctors
        //=================================================

        template<class It>
        Unrolled_linked_list(It it0, It it1);

        Unrolled_linked_list();
        Unrolled_linked_list(const Unrolled_linked_list&);
        Unrolled_linked_list(Unrolled_linked_list&&);
        ~Unrolled_linked_list();

        //=================================================
        // Assignment operators
        //=================================================

        Unrolled_linked_list& operator=(const Unrolled_linked_list&);
        Unrolled_linked_list& operator=(Unrolled_linked_list&&);

        //=================================================
        // Iterator methods
        //=================================================

        iterator begin() noexcept {
            return iterator{head, 0};
        }

        const_iterator begin() const {
            return const_iterator{head, 0};
        }

        const_iterator cbegin() const {
            return const_cast<const Unrolled_linked_list&>(*this).begin();
        }

        iterator end() noexcept;

        const_iterator end() const;

        const_iterator cend() const {
            return const_cast<const Unrolled_linked_list&>(*this).end();
        }

        //=================================================
        // Mutators
        //=================================================

        void assign(size_type count, const T& value);

        template<class InputIt>
        void assign(InputIt first, InputIt begin);

        void assign(std::initializer_list<T> list) {
            assign(list.begin(), list.end());
        }

        void clear() noexcept {
            for (auto* ptr = head; ptr; ptr = ptr->next) {
                auto allocator = base::get_allocator();

                // Delete elements in node
                for (size_type i = 0; i < ptr->elem_count; ++i) {
                    std::allocator_traits<node_allocator>::destroy(allocator, *ptr[i]);
                }

                // Deallocate node
                std::allocator_traits<node_allocator>::deallocate(allocator, ptr, 1);
                delete ptr;
            }
        }

        iterator insert(const_iterator pos, const T& value);

        iterator insert(const_iterator pos, T&& value);

        iterator insert(const_iterator pos, size_type count, const T& value);

        template<class InputIt>
        iterator insert(const_iterator pos, InputIt first, InputIt last);

        iterator insert(const_iterator pos, std::initializer_list<T> list);

        template<class...Args>
        iterator emplace(const_iterator pos, Args&&...args);

        iterator erase(const_iterator pos);

        iterator erase(const_iterator first, const_iterator last);

        void push_back(const T& value);

        void push_back(T&& value);

        template<class...Args>
        reference emplace_back(Args&&...args);

        

        void pop_back();

        void push_front(const T& value);

        void push_front(T&& value);

        //=================================================
        // Accessors
        //=================================================

        reference front() {
            return *head[0];
        }

        const_reference front() const {
            return *head[0];
        }

        reference back() {
            return *tail[tail->elem_count - 1];
        }

        const_reference back() const {
            return *tail[tail->elem_count - 1];
        }

        [[nodiscard]]
        bool empty() const {
            return !head;
        }

        [[nodiscard]]
        size_type size() const {
            size_type count = 0;

            for (auto* ptr = head; ptr; ptr = ptr->next) {
                count += ptr->elem_count;
            }

            return count;
        }

        [[nodiscard]]
        size_type max_size() const {
            constexpr size_type size_type_max = std::numeric_limits<size_type>::max();

            auto allocator = get_allocator();
            constexpr size_type allocator_max = std::allocator_traits<node_allocator>::max_size(allocator);

            return std::min(size_type_max, allocator_max);
        }

    private:

        //=================================================
        // Instance members
        //=================================================

        node_pointer head = nullptr;

        node_pointer tail = nullptr;

    };

}

#endif //AUL_UNROLLED_LINKED_LIST_HPP

#ifndef AUL_LACED_TREE_HPP
#define AUL_LACED_TREE_HPP

#include "../memory/Memory.hpp"

#include <type_traits>
#include <memory>
#include <algorithm>

namespace aul {

    template<class Val_type, class Ptr_type, class Diff_type, class Node_ptr_type, bool is_const>
    class Laced_tree_iterator {
        using node_pointer = Node_ptr_type;
    public:

        using value_type = Val_type;

        using pointer = Ptr_type;

        using reference = std::conditional_t <
            is_const,
            const value_type&,
            value_type&
        >;

        using difference_type = Diff_type;
        using iterator_category = std::forward_iterator_tag;

        //=================================================
        // Constructors
        //=================================================

        Laced_tree_iterator() = default;

        Laced_tree_iterator(node_pointer ptr) :
            p(ptr) {}

        ~Laced_tree_iterator() = default;

        //=================================================
        // Comparison operators
        //=================================================

        bool operator==(const Laced_tree_iterator it) {
            return p == it.p;
        }

        bool operator!=(const Laced_tree_iterator it) {
            return p != it.p;
        }

        //=================================================
        // Increment operators
        //=================================================

        Laced_tree_iterator operator++() {
            //Traverse to next node
            if (p != p->next) {
                p = p->next();
                return *this;
            }

            //Traverse succesor nodes
            node_pointer prev = p;
            while (p->succ->next == prev) {
                p = p->succ;
            }

            return *this;
        }

        Laced_tree_iterator operator++(int) {
            auto temp = *this;
            ++(*this);
            return temp;
        }

        //=================================================
        // Dereference operators
        //=================================================

        reference operator*() const {
            return *(p->data);
        }

        pointer operator->() const {
            return p->data;
        }

    private:
        node_pointer p;
    };




    ///
    ///
    ///
    /// \tparam T     Container element type
    /// \tparam Alloc Container allocator type
    ///
    template<typename T, typename Alloc = std::allocator<T>>
    class Laced_tree {
        class Laced_tree_node;

        //=================================================
        // Type aliases
        //=================================================
    private:

        using node_allocator_type = Alloc::template rebind<Laced_tree_node>;

        using node_pointer = std::allocator_traits<node_allocator_type>::pointer;
        using const_node_pointer = std::allocator_traits<node_allocator_type>::const_pointer;

    public:

        using allocator_type = Alloc;

        using node_type = Laced_tree_node;

        using value_type = T;

        using reference = T&;
        using const_reference = const reference;

        using size_type = typename std::allocator_traits<allocator_type>::size_type;
        using difference_type = typename std::allocator_traits<allocator_type>::difference_type;

        using pointer = typename std::allocator_traits<allocator_type>::pointer;
        using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;

        using iterator = Laced_tree_iterator<value_type, pointer, difference_type, node_pointer, false>;
        using const_iterator = Laced_tree_iterator<value_type, const_pointer, difference_type, node_pointer, true>;

    public:

        //=================================================
        // Constructors
        //=================================================

        Laced_tree() = default;

        ~Laced_tree() {
            clear();
        }

        //=================================================
        // Comparison operators
        //=================================================

        bool operator==(const Laced_tree& rhs) noexcept {
            if (size() != rhs.size()) {
                return false;
            }

            auto it0 = begin();
            auto it1 = rhs.begin();


        }

        bool operator!=(const Laced_tree& rhs) noexcept;

        //=================================================
        // Iterator methods
        //=================================================

        iterator begin() {
            return iterator(root);
        }

        const_iterator begin() const {
            return const_iterator(root);
        }

        const_iterator cbegin() {
            return const_cast<const Laced_tree*>(this).begin();
        }

        iterator end();


        //=================================================
        // Element mutators
        //=================================================

        void assign();

        void insert(const_iterator pos, const value_type v) {
            const node_pointer new_node = std::allocator_traits<node_allocator_type>::allocate(allocator, v);

            //Empty container case
            if (empty() && (pos == const_iterator{})) {
                new_node->next = new_node;
                new_node->succ = new_node;
                root = new_node;
            }

            

        }

        void emplace();

        void emplace_front();

        void emplace_back();

        void push_front();

        void pop_front();

        void push_back();

        void pop_back();

        //=================================================
        // Misc. methods
        //=================================================

        ///
        /// Removes all elements
        ///
        void clear();

        friend void swap(Laced_tree& l, Laced_tree& r) noexcept {
            l.swap(r);
        }

        void swap(Laced_tree& rhs) noexcept;

        //=================================================
        // Size methods
        //=================================================

        size_type size() const {
            return elem_count;
        }

        size_type max_size() const {
            constexpr size_type size_type_max = std::numeric_limits<size_type>::max();
            const size_type allocator_max = std::allocator_traits<node_allocator_type>::max_size(allocator);
            return std::min(size_type_max, allocator_max);
        }

        bool empty() const {
            return elem_count == 0;
        }

        allocator_type get_allocator() noexcept {
            return allocator_type{allocator};
        }

    private:
        node_allocator_type allocator{};
        size_type elem_count = 0;
        node_pointer root = nullptr;
    };

    template<typename T, class A>
    class Laced_tree<T, A>::Laced_tree_node {
    public:

        //=================================================
        // Constructors
        //=================================================

        Laced_tree_node() = default;
        Laced_tree_node(const Laced_tree_node&) = default;
        Laced_tree_node(Laced_tree_node&&) = delete;
        ~Laced_tree_node() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Laced_tree_node& operator=(const Laced_tree_node&) = default;
        Laced_tree_node& operator=(Laced_tree_node&&) = delete;


    private:
        Laced_tree<T, A>::node_pointer next = nullptr;
        Laced_tree<T, A>::node_pointer succ = nullptr;
        Laced_tree<T, A>::value_type data;
    };

}

#endif
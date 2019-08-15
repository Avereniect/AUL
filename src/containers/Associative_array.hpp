#ifndef AUL_ASSOCIATIVE_LIST_HPP
#define AUL_ASSOCIATIVE_LIST_HPP

#include "../memory/Memory.hpp"
#include "Random_access_iterator.hpp"

#include <memory>
#include <type_traits>
#include <limits>

namespace aul {

    template<class K, class T, class C = std::less<K>, class Alloc = std::allocator<T>>
    class Associative_list {
    private:
        class Associative_list_base;

    public:
        
        //-------------------------------------------------
        // Type aliases
        //-------------------------------------------------

        using key_type = K;
        using value_type = T;

        using key_compare = C;

        using pointer = typename std::allocator_traits<Alloc>::pointer;
        using const_pointer = typename std::allocator_traits<Alloc>::const_pointer;

        using reference = T&;
        using const_reference = const T&;

        using size_type = typename std::allocator_traits<Alloc>::pointer;
        using difference_type = typename std::allocator_traits<Alloc>::pointer;

        using iterator = aul::Random_access_iterator<aul::Allocator_types<Alloc>, false>;
        using const_iterator = aul::Random_access_iterator<aul::Allocator_types<Alloc>, true>;

        using allocator_type = Alloc;

    private:

        using key_allocator = typename std::allocator_traits<Alloc>::template rebind<K>;

        using key_pointer = typename std::allocator_traits<key_allocator>::pointer;
        using const_key_pointer = typename std::allocator_traits<key_allocator>::pointer;

    public:

        //-------------------------------------------------
        // -ctors
        //-------------------------------------------------

        Associative_list();
        explicit Associative_list(const key_compare compare);
        explicit Associative_list(const allocator_type& allocator);

        Associative_list(const Associative_list& list);
        Associative_list(const Associative_list& list, const allocator_type& allocator);

        Associative_list(Associative_list&&);
        Associative_list(Associative_list&&, const allocator_type& allocator);

        Associative_list(const_iterator i, const_iterator j, const key_compare compare);
        Associative_list(const_iterator i, const_iterator j);

        //-------------------------------------------------
        // Assignment operators
        //-------------------------------------------------

        Associative_list& operator=(const Associative_list& rhs);

        Associative_list& operator=(Associative_list&& list);

        //-------------------------------------------------
        // Iterator methods
        //-------------------------------------------------

        iterator begin();
        const_iterator begin() const;
        const_iterator cbegin() const;

        iterator end();
        const_iterator end() const;
        const_iterator cend() const;

        //-------------------------------------------------
        // Comparison Operators
        //-------------------------------------------------

        bool operator==(const Associative_list& rhs);
        bool operator!=(const Associative_list& rhs);

        //-------------------------------------------------
        // Element access
        //-------------------------------------------------



        //-------------------------------------------------
        // Insertion & emplacement methods
        //-------------------------------------------------

        std::pair<iterator, bool> insert(const value_type& ref);

        template<typename U>
        std::pair<iterator, bool> insert(U&&);

        std::pair<iterator, bool> insert(value_type&&);

        iterator insert(const_iterator hint, const value_type& value);

        template<typename U>
        std::pair<iterator, bool> insert(const_iterator hint, U&&);

        iterator insert(const_iterator hint, value_type&&);

        template<typename U>
        std::pair<iterator, bool> insert_or_assign(const key_type& k, U&&);

        template<typename U>
        std::pair<iterator, bool> insert_or_assign(key_type&& k, U&&);

        template<typename U>
        std::pair<iterator, bool> insert_or_assign(const_iterator hint, const key_type& k, U&&);

        template<typename U>
        std::pair<iterator, bool> insert_or_assign(const_iterator hint, key_type&& k, U&&);

        template<typename...Args>
        std::pair<iterator, bool> emplace(Args&&...args);

        template<typename...Args>
        std::pair<iterator, bool> emplace_hint(const_iterator hint, Args&&...args);

        template<typename...Args>
        std::pair<iterator, bool> try_emplace(const key_type&, Args&&...args);

        template<typename...Args>
        std::pair<iterator, bool> try_emplace(key_type&&, Args&&...args);

        template<typename...Args>
        std::pair<iterator, bool> try_emplace_hint(const_iterator hint, const key_type&, Args&&...args);

        template<typename...Args>
        std::pair<iterator, bool> try_emplace_hint(const_iterator hint, key_type&&, Args&&...args);

        //-------------------------------------------------
        // Erasure methods
        //-------------------------------------------------

        iterator erase(iterator pos);

        iterator erase(const_iterator i, const_iterator j);

        size_type erase(const key_type& key);

        //-------------------------------------------------
        // Lookup functions
        //-------------------------------------------------

        size_type count(const key_type& key);

        template<typename U>
        size_type count(const U&);

        iterator find(const key_type& key);
        const_iterator find(const key_type& key) const;

        template<typename U>
        iterator find(const U& key);

        template<typename U>
        const_iterator find(const U& key) const;

        bool contains(const key_type& key)const;

        template<typename U>
        bool contains(const U& key) const;

        //-------------------------------------------------
        // Misc. methods
        //-------------------------------------------------

        void swap(Associative_list& rhs);

        allocator_type get_allocate() const;

        //-------------------------------------------------
        // Size/capacity methods
        //-------------------------------------------------

        inline size_type size() const noexcept {
            return base.size;
        }

        inline size_type max_size() const noexcept {
            constexpr size_type a = std::numeric_limits<size_type>::max();
            size_type b = std::allocator_traits<Alloc>::max_size(base.alloc);

            return std::min(a, b);
        }

        inline bool empty() const noexcept {
            return begin() == end();
        }

        void clear() noexcept;

        //-------------------------------------------------
        // Internal details
        //-------------------------------------------------

        key_compare key_comp() const {
            return base.compare;
        }

    private:

        Associative_list_base base;

        class Associative_list_base {
        public:

            Associative_list_base();
            ~Associative_list_base();

        private:

            //---------------------------------------------
            // Instance variables
            //---------------------------------------------

            allocator_type val_alloc{};
            key_allocator  key_alloc{};

            size_type size{};
            size_type capacity{};

            pointer values{};
            key_pointer keys{};

            const key_compare compare;

            //---------------------------------------------
            // Helper functions
            //---------------------------------------------



        };

    };

    template<class K, class T, typename A>
    void swap(Associative_list<K, T, A> lhs, Associative_list<K, T, A> rhs);

}

#endif //AUL_ASSOCIATIVE_LIST_HPP
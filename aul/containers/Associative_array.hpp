#ifndef AUL_ASSOCIATIVE_ARRAY_HPP
#define AUL_ASSOCIATIVE_ARRAY_HPP

#include "../memory/Memory.hpp"
#include "Random_access_iterator.hpp"

#include <memory>
#include <type_traits>
#include <limits>
#include <algorithm>

namespace aul {

    template<class K, class T, class C = std::less<K>, class Alloc = std::allocator<T>>
    class Associative_array {
    private:
        class Associative_array_base;

    public:

        //-------------------------------------------------
        // Type aliases
        //-------------------------------------------------

        using allocator_type = template std::allocator_traits<Alloc>::template rebind_alloc<T>;

        using key_type = K;
        using value_type = T;

        using key_compare = C;

        using pointer = typename std::allocator_traits<allocator_type>::pointer;
        using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;

        using reference = T&;
        using const_reference = const T&;

        using size_type = typename std::allocator_traits<allocator_type>::size_type;
        using difference_type = typename std::allocator_traits<allocator_type>::pointer;

        using iterator = aul::Random_access_iterator<aul::Allocator_types<Alloc>, false>;
        using const_iterator = aul::Random_access_iterator<aul::Allocator_types<Alloc>, true>;


    private:

        using val_allocator_type = allocator_traits;
        using key_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<K>;

        using val_pointer = pointer;
        using const_val_pointer = const_pointer;

        using key_pointer = typename std::allocator_traits<key_allocator_type>::pointer;
        using const_key_pointer = typename std::allocator_traits<key_allocator_type>::pointer;

        using key_alloc_traits = std::allocator_traits<key_allocator_type>;
        using val_alloc_traits = std::allocator_traits<allocator_type>;

        static_assert(std::is_same<
            key_alloc_traits::difference_type,
            val_alloc_traits::difference_type
        >value);

        static_assert(std::is_same<
            key_alloc_traits::size_type,
            val_alloc_traits::size_type
        >::value);

    public:

        //-------------------------------------------------
        // -ctors
        //-------------------------------------------------

        Associative_array():
            base(0, key_compare{}, allocator_type{})
        {}

        explicit Associative_array(const size_type n):
            base(n, key_compare{}, allocator_type{})
        {}

        Associative_array(const size_type n, const key_compare compare):
            base(n, compare, allocator_type{})
        {}

        Associative_array(const size_type n, const allocator_type allocator):
            base(n, key_compare{}, allocator)
        {}

        Associative_array(const size_type n, const key_compare compare, const allocator_type& allocator):
            base(n, compare, allocator)
        {}

        explicit Associative_array(const key_compare compare):
            base(0, compare, allocator_type{})
        {}

        Associative_array(const key_compare compare, const allocator_type& allocator):
            base(0, compare, allocator)
        {}

        explicit Associative_array(const allocator_type& allocator):
            base(0, key_compare{}, allocator)
        {}

        Associative_array(const Associative_array& arr):
            base(arr.base)
        {
            aul::uninitialized_copy_n(arr.base.values, base.size, base.values, base.val_alloc);
            aul::uninitialized_copy_n(arr.base.keys, base.size, base.key, base.key_alloc);
        }

        Associative_array(const Associative_array& arr, const allocator_type& allocator):
            base(arr.base, allocator)
        {}

        Associative_array(Associative_array&& arr):
            base(std::move(arr.base))
        {}

        Associative_array(Associative_array&& arr, const allocator_type& allocator):
            base(std::move(arr.base), allocator)
        {}

        //-------------------------------------------------
        // Assignment operators
        //-------------------------------------------------

        Associative_array& operator=(const Associative_array& rhs);

        Associative_array& operator=(Associative_array&& list);

        //-------------------------------------------------
        // Iterator methods
        //-------------------------------------------------

        inline iterator begin() noexcept {
            return iterator{base.values};
        }

        inline const_iterator begin() const noexcept {
            return const_iterator{base.values};
        }

        inline const_iterator cbegin() const noexcept {
            return const_iterator{base.values};
        }

        inline iterator end() noexcept {
            return iterator{base.values + base.size};
        }

        inline const_iterator end() const noexcept {
            return const_iterator{base.values + base.size};
        }

        inline const_iterator cend() const noexcept {
            return const_iterator{base.values + base.size};
        }

        //-------------------------------------------------
        // Comparison Operators
        //-------------------------------------------------

        bool operator==(const Associative_array& rhs) {
            return (size() == rhs.size()) ? std::equal(begin(), end(), rhs.begin()) : false;
        }

        bool operator!=(const Associative_array& rhs) {
            return (size() != rhs.size()) ? true : !(*this == rhs);
        }

        //-------------------------------------------------
        // Element access
        //-------------------------------------------------

        T& at(const key_type& key) {
            key_pointer pos = std::lower_bound(base.keys, base.keys + base.size, key, base.compare);

            if (*pos != key) {
                return *pointer(nullptr);
            }

            return base.values[pos - base.keys];
        }

        const T& at(const key_type& key) const {
            key_pointer pos = std::lower_bound(base.keys, base.keys + base.size, key, base.compare);

            if (*pos != key) {
                return *pointer(nullptr);
            }

            return base.values[pos - base.keys];
        }


        T& at(const size_type x) {
            if (x  < size()) {
                return base.values[x];
            }

            throw std::out_of_range("aul::Associative_array::at() called with invalid index");
        }


        const T& at(const size_type x) const {
            if (x  < size()) {
                return base.values[x];
            }

            throw std::out_of_range("aul::Associative_array::at() called with invalid index");
        }


        T& operator[](const key_type& key) {
            auto[a, b] = std::equal_range(base.keys, base.keys + base.size, std::forward(key));
            return *a;
        }

        T& operator[](key_type&& key) {
            auto[a, b] = std::equal_range(base.keys, base.keys + base.size, std::forward(key));
            return *a;
        }

        T& operator[](const size_type x) {
            return base.values[x];
        }

        const T& operator[](const size_type x) const {
            return base.values[x];
        }

        //-------------------------------------------------
        // Insertion & emplacement methods
        //-------------------------------------------------

        std::pair<iterator, bool> insert(const key_type& k, const value_type& ref) {
            reserve(size() + 1);
        }

        template<typename U>
        std::pair<iterator, bool> insert(U&&, const value_type& ref);

        std::pair<iterator, bool> insert(value_type&&);

        template<typename U>
        std::pair<iterator, bool> insert_or_assign(const key_type& k, U&&);

        template<typename U>
        std::pair<iterator, bool> insert_or_assign(key_type&& k, U&&);

        template<typename...Args>
        std::pair<iterator, bool> emplace(Args&&...args);

        template<typename...Args>
        std::pair<iterator, bool> try_emplace(const key_type&, Args&&...args);

        template<typename...Args>
        std::pair<iterator, bool> try_emplace(key_type&&, Args&&...args);

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

        void swap(Associative_array& rhs) noexcept;

        allocator_type get_allocate() const {
            return base.val_alloc;
        }

        //-------------------------------------------------
        // Size/capacity methods
        //-------------------------------------------------

        inline size_type size() const noexcept {
            return base.size;
        }

        inline size_type max_size() const noexcept {
            constexpr size_type a = std::numeric_limits<size_type>::max();
            size_type b = std::allocator_traits<Alloc>::max_size(base.val_alloc);

            return std::min(a, b);
        }

        inline size_type capacity() const noexcept {
            return base.capacity;
        }

        inline bool empty() const noexcept {
            return begin() == end();
        }

        void clear() noexcept {
            aul::destroy_n(base.values, base.size, base.val_alloc);
            aul::destroy_n(base.keys,   base.size, base.key_alloc);
        }

        void reserve(const size_type n) {

        }

        //-------------------------------------------------
        // Internal details
        //-------------------------------------------------

        key_compare key_comp() const noexcept {
            return base.compare;
        }

        pointer data() const noexcept {
            return base.values;
        }

        key_pointer keys() const noexcept {
            return base.keys;
        }

    private:
        Associative_array_base base;

        //=====================================================================
        // Helper classes
        //=====================================================================

        struct Allocation {
            val_pointer vals;
            key_pointer keys;

            size_type size;
            size_type capacity;

            Allocation() = default;
            Allocation(const Allocation& alloc) = default;

            Allocation(Allocation&& alloc) {

            }

            Allocation& operator=(const Allocation&) = default;

            Allocation& operator=(Allocation&&) {

            }

            ~Allocation() = default;
        };

        struct Allocators {
            val_allocator_type val_allocator;
            key_allocator_type key_allocator;

            Allocators(const Alloc& alloc) :
                val_allocator(alloc),
                key_allocator(alloc) {
            }

            Allocators(const Allocators& allocs) :
                val_allocator(val_alloc_traits::select_on_container_copy_construction(allocs.val_allocator)),
                key_allocator(key_alloc_traits::select_on_container_copy_construction(allocs.key_allocator)) {
            }

            Allocators(Allocators&&) = default;

            Allocators& operator=(const Allocators&);

            Allocators& operator=(Allocators&&);

            void swap(Allocators& allocs) {
                if constexpr (key_alloc_traits::propogate_on_container_swap::value) {
                    std::swap(this->key_allocator, allocs.key_allocator)
                }

                if constexpr (val_alloc_traits::propogate_on_container_swap::value) {
                    std::swap(this->val_allocator, allocs.val_allocator);
                }
            }

            Allocation allocate(const size_type n);

            Allocation
        };

        class Associative_array_base {
        public:

            //---------------------------------------------
            // -ctors
            //---------------------------------------------

            Associative_array_base(size_type n, key_compare compare, allocator_type allocator):
                compare(compare),
                val_alloc(allocator),
                key_alloc(allocator)
            {
                if (n != 0) {
                    allocate(n);
                }
            }

            Associative_array_base(const Associative_array_base& array):
                size(0),
                compare(array.compare),
                val_alloc(val_alloc_traits::select_on_container_copy_construction(array.val_alloc)),
                key_alloc(key_alloc_traits::select_on_container_copy_construction(array.key_alloc))
            {
                allocate(array.capacity);
            }

            Associative_array_base(const Associative_array_base& array, const allocator_type& allocator):
                size(0),
                compare(array.compare),
                val_alloc(allocator),
                key_alloc(allocator)
            {
                allocate(array.capacity);
            }

            Associative_array_base(Associative_array_base&& list):
                size(std::move(list.size)),
                capacity(std::move(list.capacity)),
                values(std::move(list.values)),
                keys(std::move(list.keys)),
                compare(std::move(list.compare)),
                val_alloc(std::move(val_alloc)),
                key_alloc(std::move(key_alloc))
            {
                list.size = size_type{};
                list.capacity = size_type{};
                list.values = pointer{};
                list.keys = key_pointer{};
            }

            Associative_array_base(Associative_array_base&& list, const allocator_type& allocator):
                size(list.size),
                capacity(list.capacity),
                compare(std::move(list.compare)),
                key_alloc(allocator),
                val_alloc(allocator)
            {
                if ( (list.val_alloc == allocator) && (list.key_alloc == allocator) ) {
                    values = std::move(list.values);
                    keys = std::move(list.keys);

                    list.values = pointer{};
                    list.keys = key_pointer{};

                    list.size = 0;
                } else {
                    allocate(capacity);
                    aul::uninitialized_move(list.begin(), list.end(), this->values);
                }
            }

            ~Associative_array_base() {
                deallocate();
            }

            //---------------------------------------------
            // Assignment operators
            //---------------------------------------------

            Associative_array_base& operator=(const Associative_array_base& array) {
                deallocate();

                if constexpr (val_alloc_traits::propogate_on_container_copy_assignment::value) {
                    val_alloc = array.val_alloc;
                }

                if constexpr (key_alloc_traits::propogate_on_container_copy_assignment::value) {
                    key_alloc = array.key_alloc;
                }

                allocate(array.capacity);
                compare = array.compare;
            }

            Associative_array_base& operator=(Associative_array_base&& array) {
                deallocate();

                if constexpr (val_alloc_traits::propogate_on_container_copy_assignment::value) {
                    val_alloc = std::move(array.val_alloc);
                }

                if constexpr (key_alloc_traits::propogate_on_container_copy_assignment::value) {
                    key_alloc = std::move(array.key_alloc);
                }

                compare = std::move(array.compare);

                size = std::move(array.size);
                array.size = size_type{};

                capacity = std::move(array.capacity);
                array.capacity = size_type{};

                values = std::move(array.values);
                array.values = pointer{};

                keys = std::move(array.keys);
                array.keys = key_pointer{};
            }

            void swap(Associative_array_base& array) {
                if constexpr (val_alloc_traits::propgate_on_container_swap::value) {
                    std::swap(val_alloc, array.val_alloc);
                }

                if constexpr (key_alloc_traits::propogate_on_container_swap::value) {
                    std::swap(key_alloc, array.key_alloc);
                }

                std::swap(compare, array.compare);

                std::swap(size, array.size);
                std::swap(capacity, array.capacity);

                std::swap(values, array.values);
                std::swap(keys, array.keys);
            }

            //---------------------------------------------
            // Instance variables
            //---------------------------------------------
            size_type size{};
            size_type capacity{};

            pointer values{};
            key_pointer keys{};

            const key_compare compare;

            allocator_type val_alloc{};
            key_allocator_type  key_alloc{};

            //---------------------------------------------
            // Helper functions
            //---------------------------------------------

            void allocate(size_type n) {
                try {
                    values = val_alloc_traits::allocate(val_alloc, n);
                    keys   = key_alloc_traits::allocate(key_alloc, n);

                    capacity = n;
                } catch (...) {
                    values = pointer{};
                    keys = key_pointer{};

                    size = 0;
                    capacity = 0;

                    throw;
                }
            }

            void deallocate() {
                try {
                    val_alloc_traits::deallocate(val_alloc, values, capacity);
                    key_alloc_traits::deallocate(key_alloc, keys, capacity);

                    values = pointer{};
                    keys = key_pointer{};

                    capacity = 0;
                } catch (...) {
                    throw;
                }
            }

        };

    };

    template<class K, class T, class C, typename A>
    void swap(Associative_array<K, T, C, A> lhs, Associative_array<K, T, C, A> rhs) noexcept;

}

#endif //AUL_ASSOCIATIVE_ARRAY_HPP
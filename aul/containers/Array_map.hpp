#ifndef AUL_ARRAY_MAP_HPP
#define AUL_ARRAY_MAP_HPP

#include "../memory/Memory.hpp"
#include "Random_access_iterator.hpp"
#include "../Algorithms.hpp"

#include <memory>
#include <type_traits>
#include <limits>
#include <algorithm>
#include <functional>

namespace aul {

    ///
    /// \tparam K Key type
    /// \tparam T Element type
    /// \tparam C Comparator type
    /// \tparam Alloc Allocator type
    template<typename K, typename T, typename C = std::less<K>, typename Alloc = std::allocator<T>>
    class Array_map {
    private:

        //=================================================
        // Forward declarations
        //=================================================

        class Allocation;

    public:

        //=================================================
        // Type aliases
        //=================================================

        using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;

        using key_type = K;
        using value_type = T;

        using key_compare = C;

        using pointer = typename std::allocator_traits<allocator_type>::pointer;
        using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;

        using reference = value_type&;
        using const_reference = const value_type&;

        using size_type = typename std::allocator_traits<allocator_type>::size_type;
        using difference_type = typename std::allocator_traits<allocator_type>::pointer;

        using iterator = aul::Random_access_iterator<aul::Allocator_types<Alloc>, false>;
        using const_iterator = aul::Random_access_iterator<aul::Allocator_types<Alloc>, true>;

    private:

        using val_allocator_type = allocator_type;
        using key_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<K>;

        using key_alloc_traits = std::allocator_traits<key_allocator_type>;
        using val_alloc_traits = std::allocator_traits<allocator_type>;

        using val_pointer = pointer;
        using const_val_pointer = const_pointer;

        using key_pointer = typename key_alloc_traits::pointer;
        using const_key_pointer = typename key_alloc_traits::const_pointer;

    public:

        //=================================================
        // -ctors
        //=================================================

        ///
        /// Default constructor
        ///
        Array_map() = default;

        ///
        ///
        /// \param compare Object to use for element comparisons
        explicit Array_map(const key_compare compare) :
            allocator(),
            allocation(),
            comparator(compare) {}

        ///
        /// \param compare
        /// \param allocator
        Array_map(const key_compare compare, const allocator_type& allocator) :
            allocator(allocator),
            allocation(),
            comparator(compare) {}

        ///
        /// \param allocator
        ///
        explicit Array_map(const allocator_type& allocator) :
            allocator(allocator),
            allocation(),
            comparator() {}

        ///
        /// \param arr
        ///
        Array_map(const Array_map& arr) :
            allocator(val_alloc_traits::select_on_container_copy_construction(arr.allocator)),
            allocation(allocate(arr.size())),
            comparator(arr.comparator) {

            aul::uninitialized_copy_n(arr.allocation.vals , elem_count, allocation.vals, allocator);
            aul::uninitialized_copy_n(arr.allocation.keys, elem_count, allocation.keys, key_allocator_type{allocator});
        }

        ///
        /// \param arr
        /// \param allocator
        Array_map(const Array_map& arr, const allocator_type& allocator) :
            allocator(allocator),
            allocation(allocate(arr.size())),
            comparator(arr.comparator) {}

        ///
        /// \param arr
        ///
        Array_map(Array_map&& arr) :
            allocator(std::move(arr.allocator)),
            allocation(std::move(arr.allocation)),
            comparator(std::move(arr.comparator)) {}

        ///
        /// \param arr
        /// \param alloc
        Array_map(Array_map&& arr, const allocator_type& alloc) :
            allocator(alloc),
            allocation(arr.allocator != alloc ? allocate(arr.size()) : std::move(allocation)),
            comparator(std::move(arr.comparator)) {

            if (arr.allocator != alloc) {
                aul::uninitialized_move_n(arr.allocation.vals, arr.size(), allocation.vals, allocator);
                aul::uninitialized_move_n(arr.allocation.keys, arr.size(), allocation.keys, key_allocator_type{allocator});
            }
        }

        //=================================================
        // Assignment operators
        //=================================================

        Array_map& operator=(const Array_map& rhs) {
            clear();

            comparator = rhs.comparator;
            elem_count = rhs.elem_count;
            allocator = rhs.allocator;
            allocation = allocate(elem_count);

            aul::uninitialized_copy_n(allocation.vals, elem_count, rhs.allocation.vals, allocator);
            aul::uninitialized_copy_n(allocation.keys, elem_count, rhs.allocation.keys, key_allocator_type{allocator});
        }

        Array_map& operator=(Array_map&& rhs) {
            allocation = std::move(rhs.allocation);
            allocator = std::move(rhs.allocator);
            elem_count = std::move(rhs.elem_count);
            comparator = std::move(comparator);

            rhs.clear();
        }

        //=================================================
        // Iterator methods
        //=================================================

        inline iterator begin() noexcept {
            return iterator{allocation.vals ? allocation.vals : nullptr};
        }

        inline const_iterator begin() const noexcept {
            return const_iterator{allocation.vals ? allocation.vals : nullptr};
        }

        inline const_iterator cbegin() const noexcept {
            return const_cast<const Array_map&>(*this).begin();
        }

        inline iterator end() noexcept {
            return iterator{allocation.vals ? allocation.vals + elem_count : nullptr};
        }

        inline const_iterator end() const noexcept {
            return const_iterator{allocation.vals ? allocation.vals + elem_count : nullptr};
        }

        inline const_iterator cend() const noexcept {
            return const_cast<const Array_map&>(*this).end();
        }

        //=================================================
        // Comparison Operators
        //=================================================

        bool operator==(const Array_map& rhs) {
            return (size() == rhs.size()) ? std::equal(begin(), end(), rhs.begin()) : false;
        }

        bool operator!=(const Array_map& rhs) {
            return (size() != rhs.size()) ? true : !(*this == rhs);
        }

        //=================================================
        // Element access
        //=================================================

        T& at(const key_type& key) {
            key_pointer pos = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);

            if (!elem_count || !pos || pos != (allocation.keys + elem_count) || *pos != key) {
                throw std::out_of_range("aul::Array_map::at() called with invalid key");
            }

            return allocation.vals[pos - allocation.keys];
        }

        const T& at(const key_type& key) const {
            key_pointer pos = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);

            if (!pos || pos != (allocation.keys + elem_count) || *pos != key) {
                throw std::out_of_range("aul::Array_map::at() called with invalid key");
            }

            return allocation.vals[pos - allocation.keys];
        }

        /*
        T& at(const size_type x) {
            if (x < elem_count) {
                return allocation.values[x];
            }

            throw std::out_of_range("aul::Array_map::at() called with invalid index");
        }


        const T& at(const size_type x) const {
            if (x < elem_count) {
                return allocation.values[x];
            }

            throw std::out_of_range("aul::Array_map::at() called with invalid index");
        }
        */

        T& operator[](const key_type& key) {
            key_pointer key_ptr = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);
            return allocation.vals[key_ptr - allocation.keys];
        }

        T& operator[](key_type&& key) {
            key_pointer key_ptr = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);
            return allocation.vals[key_ptr - allocation.keys];
        }

        /*
        T& operator[](const size_type x) {
            return allocation.values[x];
        }

        const T& operator[](const size_type x) const {
            return allocation.values[x];
        }
        */

        //=================================================
        // Insertion & emplacement methods
        //=================================================

        ///
        /// 
        ///
        /// \param k
        /// \param ref
        /// \return
        std::pair<iterator, bool> insert(const key_type& key, const value_type& val) {
            grow(elem_count + 1);

            key_pointer key_end = allocation.keys + elem_count;
            val_pointer val_end = allocation.vals + elem_count;

            key_pointer new_key_pos = aul::binary_search(allocation.keys, key_end, key, comparator);
            val_pointer new_val_pos = allocation.vals + (new_key_pos - allocation.keys);

            if (*new_key_pos == key) {
                return std::make_pair(iterator{new_val_pos}, false);
            }

            if (new_key_pos != key_end) {
                //Move construct last elements
                val_alloc_traits::construct(allocator, val_end, val_end[-1]);
                auto alloc = key_allocator_type{allocator};
                key_alloc_traits::construct(alloc, key_end, key_end[-1]);

                //Move assign elements right
                std::move(new_key_pos, key_end - 1, new_key_pos + 1);
                std::move(new_val_pos, val_end - 1, new_val_pos + 1);
            }

            construct_val(new_val_pos, val);
            construct_key(new_key_pos, key);

            ++elem_count;

            return std::make_pair(iterator{new_val_pos}, true);
        }

        ///
        /// \tparam U
        /// \param k
        /// \return
        template<class K0 = key_type, class T0 = value_type>
        std::pair<iterator, bool> insert_or_assign(const K0&& key, const T0&& val) {
            static_assert(std::is_same_v<K0, key_type>);
            static_assert(std::is_same_v<T0, value_type>);

            grow(elem_count + 1);

            key_pointer key_end = allocation.keys + allocation.elem_count;
            val_pointer val_end = allocation.vals + allocation.elem_count;

            key_pointer new_key_pos = aul::binary_search(allocation.keys, key_end, std::forward(key), comparator);
            val_pointer new_val_pos = allocation.vals + (new_key_pos - allocation.keys);

            if (*new_key_pos == key) {
                *new_val_pos = val;
                return {{new_val_pos}, false};
            }

            if (new_key_pos != key_end) {
                //Move construct last elements
                construct_key(key_end, key_end[-1]);
                construct_val(val_end, val_end[-1]);

                //Move assign elements right
                aul::uninitialized_move(new_key_pos, key_end - 1, new_key_pos + 1);
                aul::uninitialized_move(new_val_pos, val_end - 1, new_key_pos + 1);
            }

            construct_key(new_key_pos, std::forward(key));
            construct_val(new_val_pos, std::forward(val));

            return {{new_val_pos}, true};
        }

        ///
        /// \tparam Args
        /// \param args
        /// \return
        template<class...Val_args, class K0 = key_type>
        std::pair<iterator, bool> emplace(K0&& key, Val_args&&...val_args) {
            static_assert(std::is_same_v<K0, key_type>);

            grow(elem_count + 1);

            key_pointer key_end = allocation.keys + allocation.elem_count;
            val_pointer val_end = allocation.vals + allocation.elem_count;

            key_pointer new_key_pos = aul::binary_search(allocation.keys, key_end, std::forward(key), comparator);
            val_pointer new_val_pos = allocation.vals + (new_key_pos - allocation.keys);

            if (*new_key_pos == key) {
                return {{new_val_pos}, false};
            }

            if (new_key_pos != key_end) {
                //Move construct last elements
                construct_key(key_end, key_end[-1]);
                construct_val(val_end, val_end[-1]);

                //Move assign elements right
                aul::uninitialized_move(new_key_pos, key_end - 1, new_key_pos + 1);
                aul::uninitialized_move(new_val_pos, val_end - 1, new_key_pos + 1);
            }

            construct_key(new_key_pos, std::forward(key));
            construct_val(new_val_pos, std::forward(val_args)...);

            return {{new_val_pos}, true};
        }

        //=================================================
        // Erasure methods
        //=================================================

        ///
        /// \param pos Iterator to element
        /// \return
        iterator erase(iterator pos) {
            val_pointer val_ptr = std::addressof(*pos);
            key_pointer key_ptr = allocation.keys + (val_ptr - allocation.vals);

            std::move(val_ptr + 1, allocation.vals + elem_count, val_ptr);
            std::move(key_ptr + 1, allocation.keys + elem_count, key_ptr);

            destroy_val(val_ptr + elem_count - 1);
            destroy_key(key_ptr + elem_count - 1);

            --elem_count;
            return pos;
        }

        ///
        /// \param i
        /// \param j
        /// \return
        iterator erase(const_iterator i, const_iterator j) {
            val_pointer val_ptr0 = const_cast<val_pointer>(std::addressof(*i));
            val_pointer val_ptr1 = const_cast<val_pointer>(std::addressof(*j));
            val_pointer val_end = allocation.vals + elem_count;

            key_pointer key_ptr0 = allocation.keys + (val_ptr0 - allocation.vals);
            key_pointer key_ptr1 = allocation.keys + (val_ptr1 - allocation.vals);
            key_pointer key_end = allocation.keys + elem_count;

            size_type n = (j - i);

            //Move elements [j over
            std::move(val_ptr1, val_end, val_ptr0);
            std::move(key_ptr1, key_end, key_ptr0);

            //Destroy last n elements
            aul::destroy(val_end - n, val_end, allocator);
            aul::destroy(key_end - n, key_end, key_allocator_type{allocator});
        }

        ///
        /// \param key
        /// \return
        size_type erase(const key_type& key);

        //=================================================
        // Lookup functions
        //=================================================

        size_type count(const key_type& key) {
            key_pointer ptr = aul::binary_search(allocation.keys, allocation.keys + elem_count, key);
            return (*ptr == key);
        }

        iterator find(const key_type& key) {
            key_pointer ptr = {aul::binary_search(allocation.keys, allocation.keys + elem_count, key)};
            return (*ptr == key) ? iterator{allocation.vals + (ptr - allocation.keys)} : end();
        }

        const_iterator find(const key_type& key) const {
            key_pointer ptr = {aul::binary_search(allocation.keys, allocation.keys + elem_count, key)};
            return (*ptr == key) ? iterator{allocation.vals + (ptr - allocation.keys)} : end();
        }

        bool contains(const key_type& key) const {
            key_pointer ptr = aul::binary_search(allocation.keys, allocation.keys + elem_count, key);
            return (*ptr == key);
        }

        //=================================================
        // Misc. methods
        //=================================================

        ///
        /// \param rhs
        void swap(Array_map& rhs) noexcept {
            std::swap(allocator, rhs.allocator);
            std::swap(elem_count, rhs.elem_count);
            std::swap(allocation, rhs.allocation);
            std::swap(comparator, rhs.comparator);
        }

        ///
        /// \return
        allocator_type get_allocator() const {
            return allocator;
        }

        //=================================================
        // Size/capacity methods
        //=================================================

        ///
        /// \return
        inline size_type size() const noexcept {
            return elem_count;
        }

        ///
        /// \return
        inline size_type max_size() const noexcept {
            constexpr size_type a = std::numeric_limits<size_type>::max();
            size_type b = std::allocator_traits<Alloc>::max_size(allocator);
            return std::min(a, b);
        }

        ///
        /// \return
        inline size_type capacity() const noexcept {
            return allocation.capacity;
        }

        ///
        /// \return
        inline bool empty() const noexcept {
            return !elem_count;
        }

        ///
        ///
        ///
        void clear() noexcept {
            aul::destroy_n(allocation.vals, elem_count, allocator);
            aul::destroy_n(allocation.keys, elem_count, key_allocator_type{allocator});
        }

        ///
        /// \param n
        ///
        void reserve(const size_type n) {
            if (n <= elem_count) {
                return;
            }

            if (max_size() < n) {
                throw std::runtime_error("Array_map grew beyond max size.");
            }

            Allocation new_allocation = allocate(n, allocation);
            {
                if (new_allocation.vals != allocation.vals) {
                    aul::uninitialized_move(allocation.vals, allocation.vals + elem_count, new_allocation.vals, allocator);
                }

                if (new_allocation.keys != allocation.keys) {
                    key_allocator_type alloc{allocator};
                    aul::uninitialized_move(allocation.keys, allocation.keys + elem_count, new_allocation.keys, alloc);
                }
            }

            allocation = std::move(new_allocation);
        }

        //=================================================
        // Internal details
        //=================================================

        ///
        /// \return
        key_compare key_comp() const noexcept {
            return comparator;
        }

        ///
        /// \return
        pointer data() const noexcept {
            return allocation.vals;
        }

        ///
        /// \return
        key_pointer keys() const noexcept {
            return allocation.keys;
        }

    private:

        //=================================================
        // Instance members
        //=================================================

        Alloc allocator{};

        Allocation allocation{};

        key_compare comparator{};

        size_type elem_count{};

        //=================================================
        // Helper functions
        //=================================================

        ///
        /// \param n
        /// \return
        Allocation allocate(const size_type n) {
            Allocation allocation{};

            try {
                allocation.vals = val_alloc_traits::allocate(allocator, n);
                allocation.keys = key_alloc_traits::allocate(key_allocator_type{allocator}, n);
            } catch (std::bad_alloc e) {
                allocation = Allocation{};
                throw;
            }

            allocation.capacity = n;

            return allocation;
        }

        Allocation allocate(const size_type n, const Allocation& hint) {
            Allocation allocation{};

            try {
                allocation.vals = val_alloc_traits::allocate(allocator, n, hint.vals);
                key_allocator_type alloc{allocator};
                allocation.keys = key_alloc_traits::allocate(alloc, n, hint.keys);
            } catch (std::bad_alloc e) {
                allocation = Allocation{};
            }

            allocation.capacity = n;

            return allocation;
        }

        void deallocate(Allocation& alloc) {
            val_alloc_traits::deallocate(allocator, alloc.vals, allocation.capacity);
            key_alloc_traits::deallocate(key_allocator_type{allocator}, alloc.keys, allocation.capacity);

            alloc = Allocation{};
        }

        template<class...Args>
        void construct_val(val_pointer ptr, Args&&...args) {
            val_alloc_traits::construct(allocator, ptr, std::forward<Args>(args)...);
        }

        template<class...Args>
        void construct_key(key_pointer ptr, Args&&...args) {
            key_allocator_type alloc{allocator};
            key_alloc_traits::construct(alloc, ptr, std::forward<Args>(args)...);
        }

        void destroy_val(val_pointer ptr) {
            val_alloc_traits::destroy(allocator, ptr);
        }

        void destroy_key(key_pointer ptr) {
            key_allocator_type alloc{allocator};
            key_alloc_traits::destroy(alloc, ptr);
        }

        void grow(const size_type n) {
            if (n < allocation.capacity) {
                return;
            }

            size_type new_size = (max_size() / 2 < n) ? max_size() : 2 * allocation.capacity;

            reserve(std::max(n, new_size));
        }

    };

    template<typename K, typename T, typename C, typename A>
    class Array_map<K, T, C, A>::Allocation {
    public:

        //=================================================
        // Instance members
        //=================================================

        val_pointer vals = nullptr;
        key_pointer keys = nullptr;

        size_type capacity = 0;

        //=================================================
        // -ctors
        //=================================================

        Allocation() = default;
        Allocation(const Allocation& alloc) = delete;

        Allocation(Allocation&& alloc) noexcept:
            vals(alloc.vals),
            keys(alloc.keys),
            capacity(alloc.capacity) {

            alloc = Allocation{};
        }

        ~Allocation() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Allocation& operator=(const Allocation&) = delete;

        Allocation& operator=(Allocation&& alloc) noexcept {
            capacity = std::move(alloc.capacity);
            vals = std::move(alloc.vals);
            keys = std::move(alloc.keys);

            alloc.vals = nullptr;
            alloc.keys = nullptr;
            alloc.capacity = 0;

            return *this;
        }

    };

}

#endif //AUL_ARRAY_MAP_HPP

#ifndef AUL_ARRAY_MAP_HPP
#define AUL_ARRAY_MAP_HPP

#include "Random_access_iterator.hpp"
#include "Allocator_aware_base.hpp"

#include "../memory/Memory.hpp"
#include "../Algorithms.hpp"

#include <memory>
#include <type_traits>
#include <limits>
#include <algorithm>
#include <functional>
#include <tuple>
#include <stdexcept>
#include <span>

namespace aul {

    ///
    /// An associative container implemented using two separate arrays for keys
    /// and values.
    ///
    /// \tparam K Key type
    /// \tparam T Element type
    /// \tparam C Comparator type
    /// \tparam A Allocator type
    template<typename K, typename T, typename C = std::less<K>, typename A = std::allocator<T>>
    class Array_map : public Allocator_aware_base<A> {
    private:

        //=================================================
        // Forward declarations
        //=================================================

        class Allocation;
        class Value_comparator;

    public:

        //=================================================
        // Type aliases
        //=================================================

        using allocator_type = typename std::allocator_traits<A>::template rebind_alloc<T>;

        using key_type = K;
        using value_type = T;

        using key_compare = C;
        using value_compare = Value_comparator;

        using pointer = typename std::allocator_traits<allocator_type>::pointer;
        using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;

        using reference = T&;
        using const_reference = const T&;

        using size_type = typename std::allocator_traits<allocator_type>::size_type;
        using difference_type = typename std::allocator_traits<allocator_type>::pointer;

        using iterator = aul::Random_access_iterator<pointer>;
        using const_iterator = aul::Random_access_iterator<const_pointer>;

    private:

        using base = Allocator_aware_base<A>;

        using val_allocator_type = allocator_type;
        using key_allocator_type = typename std::allocator_traits<A>::template rebind_alloc<K>;

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
        /// Constructs an empty Array_map
        ///
        Array_map() noexcept(noexcept(A{})) = default;

        ///
        /// \param compare Comparator to use for element comparisons
        /// \param allocator Allocator object to copy
        explicit Array_map(const key_compare compare, const allocator_type& alloc = {}):
            base{alloc},
            comparator{compare} {}

        ///
        /// \param allocator Allocator object to copy
        ///
        explicit Array_map(const allocator_type& allocator):
            base{allocator} {}

        ///
        /// If copying fails for any reason, the newly constructed Array_map
        /// in an empty state.
        ///
        /// \param arr Object to copy
        Array_map(const Array_map& arr):
            base{arr.get_allocator()},
            allocation{allocate(arr.elem_count)},
            comparator{arr.comparator},
            elem_count{arr.elem_count} {

            try {
                copy_elements(arr.allocation, allocation, elem_count);
            } catch(...) {
                deallocate(allocation);
                elem_count = 0;
            }
        }

        ///
        /// \param arr Object to copy
        /// \param allocator Allocator to copy for copy of arr
        Array_map(const Array_map& arr, const allocator_type& allocator):
            base{allocator},
            allocation(allocate(arr.elem_count)),
            comparator(arr.comparator),
            elem_count(arr.elem_count) {

            try {
                copy_elements(arr.allocation, allocation, elem_count);
            } catch(...) {
                deallocate(allocation);
                elem_count = 0;
            }
        }

        ///
        /// Transfers ownership of elements to new Array_map object. Moved-from
        /// object is left in empty state.
        ///
        /// \param arr Object to move from
        ///
        Array_map(Array_map&& arr) noexcept:
            base{arr.get_allocator()},
            allocation{std::move(arr.allocation)},
            comparator{std::move(arr.comparator)},
            elem_count(std::exchange(arr.elem_count, 0)) {}

        ///
        /// Allocator-extended move constructor. If new allocator does not
        /// compare equal to the allocator from the moved-from object, then
        /// the elements will be moved to a new allocation
        ///
        /// \param arr Object to move from
        /// \param alloc Allocator to copy-construct new allocator from
        Array_map(Array_map&& arr, const allocator_type& alloc) noexcept:
            base{alloc},
            allocation{(arr.allocator != alloc) ? allocate(elem_count) : std::move(allocation)},
            comparator{std::move(arr.comparator)},
            elem_count{arr.elem_count} {

            if (arr.allocator != alloc) {
                move_elements(alloc.allocation, allocation);
            }

            arr.elem_count = 0;
        }

        //=================================================
        // Assignment operators
        //=================================================

        ///
        /// Provides the strong exception guarantee
        ///
        /// \param rhs Object to copy
        /// \return *this
        Array_map& operator=(const Array_map& rhs) {
            // Attempt to create new allocation. Return if allocation failed
            Allocation new_allocation = allocate(rhs.elem_count);

            // Attempt to copy over objects. Return if an exception is thrown
            try {
                copy_elements(rhs.allocation, new_allocation, rhs.elem_count);
            } catch (...) {
                deallocate(new_allocation);
            }

            // Destroy current allocation and elements
            destroy_elements(allocation, elem_count);
            deallocate(allocation);

            // Replace current allocation with new allocation
            allocation = std::move(new_allocation);

            // Copy over other members
            base::operator=(rhs);
            comparator = rhs.comparator;
            elem_count = rhs.elem_count;

            return *this;
        }

        ///
        /// \param rhs Object to move from
        /// \return *this
        Array_map& operator=(Array_map&& rhs) noexcept(aul::is_noexcept_movable_v<A>) {
            //If allocator is not propagated and allocators do not compare equal a new allocation must be made.
            constexpr bool use_new_allocation = (!aul::is_noexcept_movable_v<A> && (base::operator!=(rhs)));
            if (use_new_allocation) {
                Allocation new_allocation = allocate(rhs.elem_count);

                move_elements(rhs.allocation, new_allocation, rhs.elem_count);
                destroy_elements(rhs.allocation, rhs.elem_count);
                rhs.deallocate(allocation);

                elem_count = rhs.elem_count;
                comparator = std::move(rhs.comparator);
                allocation = std::move(rhs.allocation);

                rhs.elem_count = 0;

            } else {
                base::operator=(rhs);

                allocation = std::move(rhs.allocation);
                comparator = rhs.comparator;
                elem_count = rhs.elem_count;

                rhs.elem_count = 0;
            }

            return *this;
        }

        //=================================================
        // Iterator methods
        //=================================================

        [[nodiscard]]
        iterator begin() noexcept {
            return iterator{allocation.vals};
        }

        [[nodiscard]]
        const_iterator begin() const noexcept {
            return const_iterator{allocation.vals};
        }

        [[nodiscard]]
        const_iterator cbegin() const noexcept {
            return const_cast<const Array_map&>(*this).begin();
        }

        [[nodiscard]]
        iterator end() noexcept {
            return iterator{allocation.vals + elem_count};
        }

        [[nodiscard]]
        const_iterator end() const noexcept {
            return const_iterator{allocation.vals + elem_count};
        }

        [[nodiscard]]
        const_iterator cend() const noexcept {
            return const_cast<const Array_map&>(*this).end();
        }

        //=================================================
        // Comparison Operators
        //=================================================

        ///
        /// \param rhs Array_map to compare against
        /// \return True if all keys and elements compare equal
        bool operator==(const Array_map& rhs) const noexcept {
            if (elem_count != rhs.elem_count) {
                return false;
            }

            return
                std::equal(allocation.keys, allocation.keys + elem_count, rhs.allocation.keys) &&
                std::equal(allocation.vals, allocation.vals + elem_count, rhs.allocation.vals);
        }

        ///
        /// \param rhs Array_map to compare against
        /// \return True if at least one key or element does not compare equal
        bool operator!=(const Array_map& rhs) const noexcept {
            if (elem_count != rhs.elem_count) {
                return true;
            }

            return
                !std::equal(allocation.keys, allocation.keys + elem_count, rhs.allocation.keys) &&
                !std::equal(allocation.vals, allocation.vals + elem_count, rhs.allocation.vals);
        }

        //=================================================
        // Element access
        //=================================================

        [[nodiscard]]
        T& at(const key_type& key) {
            const key_pointer pos = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);

            if (!elem_count || !pos || pos == (allocation.keys + elem_count) || *pos != key) {
                throw std::out_of_range("aul::Array_map::at() called with invalid key");
            }

            return allocation.vals[pos - allocation.keys];
        }

        [[nodiscard]]
        const T& at(const key_type& key) const {
            const key_pointer pos = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);

            if (!elem_count || !pos || pos == (allocation.keys + elem_count) || *pos != key) {
                throw std::out_of_range("aul::Array_map::at() called with invalid key");
            }

            return allocation.vals[pos - allocation.keys];
        }

        [[nodiscard]]
        T& at(key_type&& key) {
            const key_pointer pos = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);

            if (!elem_count || !pos || pos == (allocation.keys + elem_count) || *pos != key) {
                throw std::out_of_range("aul::Array_map::at() called with invalid key");
            }

            return allocation.vals[pos - allocation.keys];
        }

        [[nodiscard]]
        const T& at(const key_type&& key) const {
            const key_pointer pos = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);

            if (!elem_count || !pos || pos == (allocation.keys + elem_count) || *pos != key) {
                throw std::out_of_range("aul::Array_map::at() called with invalid key");
            }

            return allocation.vals[pos - allocation.keys];
        }



        [[nodiscard]]
        reference operator[](const key_type& key) noexcept {
            const key_pointer key_ptr = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);
            return allocation.vals[key_ptr - allocation.keys];
        }

        [[nodiscard]]
        const_reference operator[](const key_type& key) const noexcept {
            const key_pointer key_ptr = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);
            return allocation.vals[key_ptr - allocation.keys];
        }

        [[nodiscard]]
        reference operator[](key_type&& key) noexcept {
            const key_pointer key_ptr = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);
            return allocation.vals[key_ptr - allocation.keys];
        }

        [[nodiscard]]
        const_reference operator[](key_type&& key) const noexcept {
            const key_pointer key_ptr = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);
            return allocation.vals[key_ptr - allocation.keys];
        }

        //=================================================
        // Insertion & emplacement methods
        //=================================================

        ///
        /// Provides the strong exception guarantee
        ///
        /// \param key Key to associate with val
        /// \param val Value
        /// \return Iterator to newly inserted element and boolean indicating
        /// whether object was inserted.
        std::pair<iterator, bool> insert(const key_type& key, const value_type& val) {
            return emplace(key, val);
        }

        std::pair<iterator, bool> insert(const key_type& key, const value_type&& val) {
            return emplace(key, std::forward(val));
        }

        std::pair<iterator, bool> insert(const key_type&& key, const value_type& val) {
            return emplace(std::forward(key), val);
        }

        std::pair<iterator, bool> insert(const key_type&& key, const value_type&& val) {
            return emplace(key, val);
        }

        ///
        /// Constructs element with an association with key
        ///
        /// Provides the strong exception guarantee
        ///
        /// \tparam Args Argument types to value constructor
        /// \param key Reference ot key
        /// \param args Arguments to value constructor
        /// \return Pair containing iterator to element and boolean indicating
        ///     whether a new element was added
        ///
        template<class...Args>
        std::pair<iterator, bool> emplace(const key_type& key, Args&&...args) {
            if (size() > max_size() - 1) {
                throw std::length_error("aul::Array_map grew too big");
            }

            //Check if element with key already exists
            key_pointer key_ptr = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);
            if (key_ptr && !empty() && compare_keys(*key_ptr, key)) {
                pointer ptr = allocation.vals + (key_ptr - allocation.keys);
                return std::make_pair(iterator{ptr}, false);
            }

            if (size() + 1 <= capacity()) {
                key_pointer keys_end = allocation.keys + elem_count;
                val_pointer vals_end = allocation.vals + elem_count;

                key_pointer new_key_ptr = key_ptr;
                val_pointer new_val_ptr = allocation.vals + (new_key_ptr - allocation.keys);

                //Move elements to open up space
                if (new_key_ptr != keys_end) {
                    //Move construct last element in each array
                    construct_val(vals_end, std::move(vals_end[-1]));
                    construct_key(keys_end, std::move(keys_end[-1]));

                    //Move assign elements 1 slot to the right
                    std::move_backward(new_key_ptr, keys_end - 1, keys_end);
                    std::move_backward(new_val_ptr, vals_end - 1, vals_end);

                    destroy_key(new_key_ptr);
                    destroy_val(new_val_ptr);
                }

                try {
                    construct_val(new_val_ptr, std::forward<Args>(args)...);
                    construct_key(new_key_ptr, key);
                } catch (...) {
                    //Move keys and vals back to their original positions
                    if (new_key_ptr != keys_end) {
                        std::move(new_key_ptr + 1, keys_end + 1, new_key_ptr);
                        std::move(new_val_ptr + 1, vals_end + 1, new_val_ptr);

                        destroy_key(keys_end);
                        destroy_val(vals_end);
                    }
                }

                ++elem_count;
                return std::make_pair(iterator{new_val_ptr}, true);
            } else {
                Allocation new_allocation = allocate(grow_size(size() + 1));

                key_pointer old_key_ptr = key_ptr;

                key_pointer new_key_ptr = new_allocation.keys + (old_key_ptr - allocation.keys);
                val_pointer new_val_ptr = new_allocation.vals + (old_key_ptr - allocation.keys);

                //Try constructing value and key
                try {
                    construct_key(new_key_ptr, std::move(key));
                } catch (...) {
                    deallocate(new_allocation);
                    throw;
                }

                try {
                    construct_val(new_val_ptr, std::forward<Args>(args)...);
                } catch (...) {
                    destroy_key(new_key_ptr);
                    deallocate(new_allocation);
                    throw;
                }

                ///Move objects keys and vals to new allocation

                auto val_alloc = get_allocator();
                auto key_alloc = key_allocator_type{val_alloc};
                aul::uninitialized_move(allocation.keys, old_key_ptr, new_allocation.keys, key_alloc);
                aul::uninitialized_move(old_key_ptr, allocation.keys + size(), new_key_ptr + 1, key_alloc);

                pointer old_val_ptr = allocation.vals + (old_key_ptr - allocation.keys);
                aul::uninitialized_move(allocation.vals, old_val_ptr, new_allocation.vals, val_alloc);
                aul::uninitialized_move(old_val_ptr, allocation.vals + size(), new_val_ptr + 1, val_alloc);

                allocation = std::move(new_allocation);

                ++elem_count;
                return std::make_pair(iterator{new_val_ptr}, true);
            }
        }

        ///
        /// Constructs element with an association with key
        ///
        /// Provides the strong exception guarantee
        ///
        /// \tparam Args Argument types to value constructor
        /// \param key Rvalue reference ot key
        /// \param args Arguments to value constructor
        /// \return Pair containing iterator to element and boolean indicating
        ///     whether a new element was added
        ///
        template<class...Args>
        std::pair<iterator, bool> emplace(key_type&& key, Args&&...args) {
            if (size() > max_size() - 1) {
                throw std::length_error("aul::Array_map grew too big");
            }

            //Check if element with key already exists
            key_pointer key_ptr = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);
            if (key_ptr && !empty() && compare_keys(*key_ptr, key)) {
                pointer ptr = allocation.vals + (key_ptr - allocation.keys);
                return std::make_pair(iterator{ptr}, false);
            }

            if (size() + 1 <= capacity()) {
                key_pointer keys_end = allocation.keys + elem_count;
                val_pointer vals_end = allocation.vals + elem_count;

                key_pointer new_key_ptr = key_ptr;
                val_pointer new_val_ptr = allocation.vals + (new_key_ptr - allocation.keys);

                //Move elements to new allocation
                if (new_key_ptr != keys_end) {
                    //Move construct last element in each array
                    construct_key(keys_end, std::move(keys_end[-1]));
                    construct_val(vals_end, std::move(vals_end[-1]));

                    //Move assign elements 1 slot to the right
                    std::move_backward(new_key_ptr, keys_end - 1, keys_end);
                    std::move_backward(new_val_ptr, vals_end - 1, vals_end);

                    destroy_key(new_key_ptr);
                    destroy_val(new_val_ptr);
                }

                try {
                    construct_val(new_val_ptr, std::forward<Args>(args)...);
                    construct_key(new_key_ptr, key);
                } catch (...) {
                    //Move keys and vals back to their original positions
                    if (new_key_ptr != keys_end) {
                        std::move(new_key_ptr + 1, keys_end + 1, new_key_ptr);
                        std::move(new_val_ptr + 1, vals_end + 1, new_val_ptr);

                        destroy_key(keys_end);
                        destroy_val(vals_end);
                    }
                }

                ++elem_count;
                return std::make_pair(iterator{new_val_ptr}, true);
            } else {
                Allocation new_allocation = allocate(grow_size(size() + 1));

                key_pointer old_key_ptr = key_ptr;

                key_pointer new_key_ptr = new_allocation.keys + (old_key_ptr - allocation.keys);
                val_pointer new_val_ptr = new_allocation.vals + (old_key_ptr - allocation.keys);

                //Try constructing value and key
                try {
                    construct_key(new_key_ptr, std::move(key));
                } catch (...) {
                    deallocate(new_allocation);
                    throw;
                }

                try {
                    construct_val(new_val_ptr, std::forward<Args>(args)...);
                } catch (...) {
                    destroy_key(new_key_ptr);
                    deallocate(new_allocation);
                    throw;
                }

                ///Move objects keys and vals to new allocation

                auto val_alloc = get_allocator();
                auto key_alloc = key_allocator_type{val_alloc};
                aul::uninitialized_move(allocation.keys, old_key_ptr, new_allocation.keys, key_alloc);
                aul::uninitialized_move(old_key_ptr, allocation.keys + size(), new_key_ptr + 1, key_alloc);

                pointer old_val_ptr = allocation.vals + (old_key_ptr - allocation.keys);
                aul::uninitialized_move(allocation.vals, old_val_ptr, new_allocation.vals, val_alloc);
                aul::uninitialized_move(old_val_ptr, allocation.vals + size(), new_val_ptr + 1, val_alloc);

                allocation = std::move(new_allocation);

                ++elem_count;
                return std::make_pair(iterator{new_val_ptr}, true);
            }
        }

        ///
        /// If an element is associated with the specified key,
        ///
        /// \tparam Args Argument types to value constructor
        /// \param key Const reference to key
        /// \param args Arguments to value constructor
        /// \return Pair containing iterator to element and boolean indicating
        ///     whether a new element was added
        template<class...Args>
        std::pair<iterator, bool> emplace_or_assign(const key_type& key, Args&&...args) {
            if (size() > max_size() - 1) {
                throw std::length_error("aul::Array_map grew too big");
            }

            //Check if element already exists
            key_pointer key_ptr = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);
            if (key_ptr && !empty() && compare_keys(*key_ptr, key)) {
                pointer element_ptr = allocation.vals + (key_ptr - allocation.keys);
                T temp{args...};
                *elem_count = temp;
                return {element_ptr, false};
            }

            if (size() + 1 <= capacity()) {
                key_pointer keys_end = allocation.keys + elem_count;
                val_pointer vals_end = allocation.vals + elem_count;

                key_pointer new_key_ptr = key_ptr;
                val_pointer new_val_ptr = allocation.vals + (new_key_ptr - allocation.keys);

                //Move elements to open up space
                if (new_key_ptr != keys_end) {
                    //Move construct last element in each array
                    construct_key(vals_end, std::move(vals_end[-1]));
                    construct_val(vals_end, std::move(keys_end[-1]));

                    //Move assign elements 1 slot to the right
                    std::move_backward(new_key_ptr, keys_end - 1, keys_end);
                    std::move_backward(new_val_ptr, vals_end - 1, vals_end);

                    destroy_key(new_key_ptr);
                    destroy_val(new_val_ptr);
                }

                try {
                    construct_val(new_val_ptr, std::forward<Args>(args)...);
                    construct_key(new_key_ptr, key);
                } catch (...) {
                    //Move keys and vals back to their original positions
                    if (new_key_ptr != keys_end) {
                        std::move(new_key_ptr + 1, keys_end + 1, new_key_ptr);
                        std::move(new_val_ptr + 1, vals_end + 1, new_val_ptr);

                        destroy_key(keys_end);
                        destroy_val(vals_end);
                    }
                }

                ++elem_count;
                return std::make_pair(iterator{new_val_ptr}, true);
            } else {
                Allocation new_allocation = allocate(grow_size(size() + 1));

                key_pointer old_key_ptr = key_ptr;

                key_pointer new_key_ptr = new_allocation.keys + (old_key_ptr - allocation.keys);
                val_pointer new_val_ptr = new_allocation.vals + (old_key_ptr - allocation.keys);

                //Try constructing value and key
                try {
                    construct_key(new_key_ptr, std::move(key));
                } catch (...) {
                    deallocate(new_allocation);
                    throw;
                }

                try {
                    construct_val(new_val_ptr, std::forward<Args>(args)...);
                } catch (...) {
                    destroy_key(new_key_ptr);
                    deallocate(new_allocation);
                    throw;
                }

                ///Move objects keys and vals to new allocation

                auto val_alloc = get_allocator();
                auto key_alloc = key_allocator_type{val_alloc};
                aul::uninitialized_move(allocation.keys, old_key_ptr, new_allocation.keys, key_alloc);
                aul::uninitialized_move(old_key_ptr, allocation.keys + size(), new_key_ptr + 1, key_alloc);

                pointer old_val_ptr = allocation.vals + (old_key_ptr - allocation.keys);
                aul::uninitialized_move(allocation.vals, old_val_ptr, new_allocation.vals, val_alloc);
                aul::uninitialized_move(old_val_ptr, allocation.vals + size(), new_val_ptr + 1, val_alloc);

                allocation = std::move(new_allocation);

                ++elem_count;
                return std::make_pair(iterator{new_val_ptr}, true);
            }
        }

        ///
        /// If an element is associated with the specified key,
        ///
        /// \tparam Args Argument types to value constructor
        /// \param key Const reference to key
        /// \param args Arguments to value constructor
        /// \return Pair containing iterator to element and boolean indicating
        ///     whether a new element was added
        template<class...Args>
        std::pair<iterator, bool> emplace_or_assign(key_type&& key, Args&&...args) {
            if (size() > max_size() - 1) {
                throw std::length_error("aul::Array_map grew too big");
            }

            //Check if element already exists
            key_pointer key_ptr = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);
            if (key_ptr && !empty() && compare_keys(*key_ptr, key)) {
                pointer element_ptr = allocation.vals + (key_ptr - allocation.keys);
                T temp{args...};
                *elem_count = temp;
                return {element_ptr, false};
            }

            if (size() + 1 <= capacity()) {
                key_pointer keys_end = allocation.keys + elem_count;
                val_pointer vals_end = allocation.vals + elem_count;

                key_pointer new_key_ptr = key_ptr;
                val_pointer new_val_ptr = allocation.vals + (new_key_ptr - allocation.keys);

                //Move elements to new allocation
                if (new_key_ptr != keys_end) {
                    //Move construct last element in each array
                    construct_key(vals_end, std::move(vals_end[-1]));
                    construct_val(vals_end, std::move(keys_end[-1]));

                    //Move assign elements 1 slot to the right
                    std::move_backward(new_key_ptr, keys_end - 1, keys_end);
                    std::move_backward(new_val_ptr, vals_end - 1, vals_end);

                    destroy_key(new_key_ptr);
                    destroy_val(new_val_ptr);
                }

                try {
                    construct_val(new_val_ptr, std::forward<Args>(args)...);
                    construct_key(new_key_ptr, key);
                } catch (...) {
                    //Move keys and vals back to their original positions
                    if (new_key_ptr != keys_end) {
                        std::move(new_key_ptr + 1, keys_end + 1, new_key_ptr);
                        std::move(new_val_ptr + 1, vals_end + 1, new_val_ptr);

                        destroy_key(keys_end);
                        destroy_val(vals_end);
                    }
                }

                ++elem_count;
                return std::make_pair(iterator{new_val_ptr}, true);
            } else {
                Allocation new_allocation = allocate(grow_size(size() + 1));

                key_pointer old_key_ptr = key_ptr;

                key_pointer new_key_ptr = new_allocation.keys + (old_key_ptr - allocation.keys);
                val_pointer new_val_ptr = new_allocation.vals + (old_key_ptr - allocation.keys);

                //Try constructing value and key
                try {
                    construct_key(new_key_ptr, std::move(key));
                } catch (...) {
                    deallocate(new_allocation);
                    throw;
                }

                try {
                    construct_val(new_val_ptr, std::forward<Args>(args)...);
                } catch (...) {
                    destroy_key(new_key_ptr);
                    deallocate(new_allocation);
                    throw;
                }

                ///Move objects keys and vals to new allocation

                auto val_alloc = get_allocator();
                auto key_alloc = key_allocator_type{val_alloc};
                aul::uninitialized_move(allocation.keys, old_key_ptr, new_allocation.keys, key_alloc);
                aul::uninitialized_move(old_key_ptr, allocation.keys + size(), new_key_ptr + 1, key_alloc);

                pointer old_val_ptr = allocation.vals + (old_key_ptr - allocation.keys);
                aul::uninitialized_move(allocation.vals, old_val_ptr, new_allocation.vals, val_alloc);
                aul::uninitialized_move(old_val_ptr, allocation.vals + size(), new_val_ptr + 1, val_alloc);

                allocation = std::move(new_allocation);

                ++elem_count;
                return std::make_pair(iterator{new_val_ptr}, true);
            }

        }

        //=================================================
        // Erasure methods
        //=================================================

        ///
        /// \param pos Valid iterator to element
        /// \return Iterator to element which replaced the removed element
        iterator erase(iterator pos) noexcept {
            val_pointer val_ptr = pos.operator->();
            key_pointer key_ptr = allocation.keys + (val_ptr - allocation.vals);

            std::move(val_ptr + 1, allocation.vals + elem_count, val_ptr);
            std::move(key_ptr + 1, allocation.keys + elem_count, key_ptr);

            destroy_val(val_ptr + elem_count - 1);
            destroy_key(key_ptr + elem_count - 1);

            --elem_count;
            return pos;
        }

        ///
        /// \param key Key mapping to element to erase
        /// \return Iterator to element which replaced the removed element.
        ///     Equal to end() if did not exist
        iterator erase(const key_type& key) noexcept {
            auto it = find(key);

            const iterator e = end();
            if (it == e) {
                return e;
            }
            return erase(it);
        }

        //=================================================
        // Inspection functions
        //=================================================

        ///
        /// \param key Key to search for
        /// \return Iterator to element corresponding to key. end() if no such
        ///     element exists
        [[nodiscard]]
        iterator find(const key_type& key) noexcept {
            key_pointer ptr = {aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator)};
            return (ptr && (*ptr == key)) ? iterator{allocation.vals + (ptr - allocation.keys)} : end();
        }

        ///
        /// \param key Key to search for
        /// \return Iterator to element. end() if not found
        [[nodiscard]]
        const_iterator find(const key_type& key) const noexcept {
            key_pointer ptr = {aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator)};
            return (ptr && (*ptr == key)) ? const_iterator{allocation.vals + (ptr - allocation.keys)} : end();
        }

        template<class K2>
        [[nodiscard]]
        iterator find(const K2& key) noexcept {
            key_pointer ptr = {aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator)};
            return (ptr && (*ptr == key)) ? iterator{allocation.vals + (ptr - allocation.keys)} : end();
        }

        template<class K2>
        [[nodiscard]]
        const_iterator find(const K2& key) const noexcept {
            key_pointer ptr = {aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator)};
            return (ptr && (*ptr == key)) ? iterator{allocation.vals + (ptr - allocation.keys)} : end();
        }

        ///
        /// \param key Key to search for
        /// \returns True if key maps to an element
        [[nodiscard]]
        bool contains(const key_type& key) const noexcept {
            key_pointer ptr = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);
            return (ptr && size() && (*ptr == key));
        }

        //=================================================
        // Misc. methods
        //=================================================

        ///
        /// \param rhs Array_map to swap contents with
        ///
        void swap(Array_map& rhs) noexcept(aul::is_noexcept_swappable_v<A>) {
            base::swap(rhs);
            std::swap(elem_count, rhs.elem_count);
            std::swap(allocation, rhs.allocation);
            std::swap(comparator, rhs.comparator);
        }

        friend void swap(Array_map& lhs, Array_map& rhs) noexcept(noexcept(lhs.swap)) {
            lhs.swap(rhs);
        }

        ///
        /// \return Copy of internal allocator
        ///
        [[nodiscard]]
        allocator_type get_allocator() const {
            return base::get_allocator();
        }

        //=================================================
        // Size/capacity methods
        //=================================================

        ///
        /// \return Number of elements held
        ///
        [[nodiscard]]
        size_type size() const noexcept {
            return elem_count;
        }

        ///
        /// \return Maximum allocation size
        ///
        [[nodiscard]]
        size_type max_size() const noexcept {
            constexpr size_type a = std::numeric_limits<size_type>::max();
            size_type b = std::allocator_traits<A>::max_size(get_allocator());
            return std::min(a, b);
        }

        ///
        /// \return Size of allocation
        ///
        [[nodiscard]]
        size_type capacity() const noexcept {
            return allocation.capacity;
        }

        ///
        /// \return True if no elements exist
        ///
        [[nodiscard]]
        bool empty() const noexcept {
            return elem_count == 0;
        }

        ///
        /// Destroys all elements and releases all memory allocations
        ///
        void clear() noexcept {
            auto val_alloc = get_allocator();
            auto key_alloc = key_allocator_type{val_alloc};
            aul::destroy_n(allocation.vals, elem_count, val_alloc);
            aul::destroy_n(allocation.keys, elem_count, key_alloc);
            elem_count = 0;
            deallocate(allocation);
        }

        ///
        /// \param n Number to increase capacity to
        ///
        void reserve(const size_type n) {
            if (n <= elem_count) {
                return;
            }

            if (max_size() < n) {
                throw std::runtime_error("Array_map grew beyond max size.");
            }

            Allocation new_allocation = allocate(n);
            move_elements(allocation, new_allocation, elem_count);

            allocation = std::move(new_allocation);
        }

        //=================================================
        // Internal details
        //=================================================

        ///
        /// \return Binary predicate object that compares keys
        ///
        [[nodiscard]]
        key_compare key_comp() const noexcept {
            return comparator;
        }

        ///
        /// /return Binary predicate object that compares element's ordering
        ///
        [[nodiscard]]
        value_compare value_comp() const noexcept {
            return value_compare{};
        }

        ///
        /// \return Pointer to internal value array
        ///
        [[nodiscard]]
        pointer data() const noexcept {
            return allocation.vals;
        }

        [[nodiscard]]
        key_pointer key_data() const noexcept {
            return allocation.keys;
        }

        #if __cplusplus >= 202002L
        ///
        /// \return Span over key array
        ///
        [[nodiscard]]
        std::span<key_type> keys() const noexcept {
            return {allocation.keys, elem_count};
        }

        [[nodiscard]]
        std::span<value_type> values() const noexcept {
            return {allocation.vals, elem_count};
        }
        #endif

    private:

        //=================================================
        // Instance members
        //=================================================

        Allocation  allocation{};
        key_compare comparator{};
        size_type   elem_count{};

        //=================================================
        // (de)allocation helper functions
        //=================================================

        Allocation allocate(const size_type n) {
            auto allocator = get_allocator();
            return allocate(allocator, n);
        }

        ///
        /// \param n Size of allocation
        /// \return Allocation of size n
        [[nodiscard]]
        static Allocation allocate(allocator_type& allocator, const size_type n) {
            Allocation ret{};

            try {
                ret.vals = val_alloc_traits::allocate(allocator, n);
                auto key_alloc = key_allocator_type{allocator};
                ret.keys = key_alloc_traits::allocate(key_alloc, n);
            } catch (...) {
                deallocate(allocator, ret);
                throw;
            }

            ret.capacity = n;

            return ret;
        }

        ///
        /// \param n Number of elements to allocate memory for
        /// \param hint Allocation to extend if possible
        /// \return Allocation of size n
        [[nodiscard]]
        static Allocation allocate(allocator_type& allocator, const size_type n, const Allocation& hint) {
            Allocation ret{};

            try {
                ret.vals = val_alloc_traits::allocate(allocator, n, hint.vals);
                key_allocator_type key_alloc{allocator};
                ret.keys = key_alloc_traits::allocate(key_alloc, n, hint.keys);
            } catch (...) {
                deallocate(allocator, ret);
                ret = Allocation{};
                throw;
            }

            ret.capacity = n;

            return ret;
        }

        void deallocate(Allocation& allocation) noexcept {

        }

        ///
        /// \param alloc Allocation to free. Empty after successful completion
        ///
        static void deallocate(allocator_type& allocator, Allocation& allocation) noexcept {
            val_alloc_traits::deallocate(allocator, allocation.vals, allocation.capacity);
            auto key_alloc = key_allocator_type{allocator};
            key_alloc_traits::deallocate(key_alloc, allocation.keys, allocation.capacity);

            allocation = Allocation{};
        }

        //=================================================
        // Construction/Destruction helper methods
        //=================================================s

        template<class...Args>
        void construct_val(val_pointer ptr, Args&&...args) {
            auto val_alloc = get_allocator();
            val_alloc_traits::construct(val_alloc, ptr, std::forward<Args>(args)...);
        }

        template<class...Args>
        void construct_key(key_pointer ptr, Args&&...args) {
            auto key_alloc = key_allocator_type{get_allocator()};
            key_alloc_traits::construct(key_alloc, ptr, std::forward<Args>(args)...);
        }

        void destroy_val(val_pointer ptr) {
            val_allocator_type val_alloc{get_allocator()};
            val_alloc_traits::destroy(val_alloc, ptr);
        }

        void destroy_key(key_pointer ptr) {
            key_allocator_type alloc{get_allocator()};
            key_alloc_traits::destroy(alloc, ptr);
        }

        ///
        /// \param source Allocation move elements from
        /// \param dest Allocation to move elements to
        /// \param n Number of elements to copy
        void move_elements(Allocation& source, Allocation& dest, size_type n) noexcept {
            auto val_alloc = get_allocator();
            auto key_alloc = key_allocator_type {val_alloc};

            aul::uninitialized_move_n(source.vals, n, dest.vals, val_alloc);
            aul::uninitialized_move_n(source.keys, n, dest.keys, val_alloc);
        }

        ///
        /// Throws if copying fails
        /// Provides the strong exception guarantee
        ///
        /// \param source Allocation to copy elements from
        /// \param dest Allocation to copy elements to
        /// \param n Number of elements to copy
        void copy_elements(const Allocation& source, Allocation& dest, size_type n) {
            auto val_alloc = get_allocator();
            auto key_alloc = key_allocator_type {val_alloc};

            try {
                aul::uninitialized_copy_n(source.vals, n, dest.vals, val_alloc);
            } catch(...) {
                throw;
            }

            try {
                aul::uninitialized_copy_n(source.keys, n, dest.keys, key_alloc);
            } catch(...) {
                aul::destroy_n(source.vals, n, val_alloc);
                throw;
            }
        }

        void destroy_elements(Allocation& source, size_type n) {
            auto val_alloc = get_allocator();
            auto key_alloc = key_allocator_type{val_alloc};

            aul::destroy_n(source.vals, n, val_alloc);
            aul::destroy_n(source.keys, n, key_alloc);
        }

        //=================================================
        // Misc. helper functions
        //=================================================

        ///
        /// \param n Minimum number of new elements
        /// \return Size the Array_map should grow to accommodate
        [[nodiscard]]
        size_type grow_size(const size_type n) const {
            size_type double_size = (n < max_size() / 2) ? 2 * elem_count : max_size();
            return std::max(double_size, n);
        }

        bool compare_keys(const key_type& a, const key_type& b) {
            return !comparator(a, b) && !comparator(b, a);
        }

    };

    template<typename K, typename T, typename C, typename A>
    class Array_map<K, T, C, A>::Value_comparator {
    public:

        ///
        /// \param x Element X
        /// \param y Element Y
        /// \return X < Y according to their key associations
        [[nodiscard]]
        bool operator()(const_iterator x, const const_iterator y) const {
            //Elements at lower addresses map to keys which are less than the
            //keys for elements at higher addresses
            return x.operator->() < y.operator->();
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

        //=================================================
        // Conversion operators
        //=================================================

        operator bool() const {
            return !capacity;
        }

    };

}

#endif //AUL_ARRAY_MAP_HPP

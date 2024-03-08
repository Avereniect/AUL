#ifndef AUL_ARRAY_MAP_HPP
#define AUL_ARRAY_MAP_HPP

#include "Zipper_iterator.hpp"
#include "Allocator_aware_base.hpp"
#include "../Span.hpp"

#include "../memory/Memory.hpp"
#include "../Algorithms.hpp"

#include <memory>
#include <type_traits>
#include <limits>
#include <algorithm>
#include <functional>
#include <tuple>
#include <stdexcept>
#include <utility>

namespace aul {

    ///
    /// An associative container implemented using two separate arrays for keys
    /// and values.
    ///
    /// \tparam K Key type
    /// \tparam V Element type
    /// \tparam C Comparator type
    /// \tparam A Allocator type
    template<typename K, typename V, typename C = std::less<K>, typename A = std::allocator<V>>
    class Array_map : public Allocator_aware_base<A> {
        using base = Allocator_aware_base<A>;

        //=================================================
        // Forward declarations
        //=================================================

        class Allocation;
        class Value_comparator;

    public:

        //=================================================
        // Type aliases
        //=================================================

        using value_type = V;
        using key_type = K;

        using value_allocator_type = A;
        using key_allocator_type = typename std::allocator_traits<A>::template rebind_alloc<key_type>;

        using value_compare = Value_comparator;
        using key_compare = C;

        using value_pointer = typename std::allocator_traits<value_allocator_type>::pointer;
        using const_value_pointer = typename std::allocator_traits<value_allocator_type>::const_pointer;

        using key_pointer = typename std::allocator_traits<typename std::allocator_traits<A>::template rebind_alloc<K>>::pointer;
        using const_key_pointer = typename std::allocator_traits<typename std::allocator_traits<A>::template rebind_alloc<K>>::const_pointer;

        using size_type = typename std::allocator_traits<value_allocator_type>::size_type;
        using difference_type = typename std::allocator_traits<value_allocator_type>::pointer;

        using iterator = aul::Random_access_zipper_iterator<key_pointer, value_pointer>;
        using const_iterator = aul::Random_access_zipper_iterator<const_key_pointer, const_value_pointer>;

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
        explicit Array_map(const key_compare compare, const value_allocator_type& alloc = {}):
            base{alloc},
            comparator{compare} {}

        ///
        /// \param allocator Allocator object to copy
        ///
        explicit Array_map(const value_allocator_type& allocator):
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
        Array_map(const Array_map& arr, const value_allocator_type& allocator):
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
        Array_map(Array_map&& arr, const value_allocator_type& alloc) noexcept:
            base{alloc},
            allocation{(arr.allocator != alloc) ? allocate(elem_count) : std::move(allocation)},
            comparator{std::move(arr.comparator)},
            elem_count{arr.elem_count} {

            if (arr.allocator != alloc) {
                move_elements(alloc.allocation, allocation);
            }

            arr.elem_count = 0;
        }

        template<class Zip_it>
        Array_map(
            Zip_it begin,
            Zip_it end,
            const key_compare compare,
            const value_allocator_type& alloc
        ):
            base{alloc},
            allocation{allocate(std::distance(begin, end))},
            comparator{std::move(compare)},
            elem_count{std::distance(begin, end)}
        {
            using std::get;

            auto values_begin = get<0>(begin);
            auto values_end = get<0>(end);
            auto value_allocator = get_allocator();
            aul::uninitialized_copy(values_begin, values_end, allocation.vals, value_allocator);

            auto keys_begin = get<1>(begin);
            auto keys_end = get<1>(end);
            auto key_alloc = key_allocator_type{get_allocator()};
            aul::uninitialized_copy(values_begin, values_end, allocation.keys, value_allocator);

            std::sort(begin(), end(), comparator);

            auto find_predicate = [&compare] (const key_type& lhs, const key_type rhs) {
                return compare(lhs, rhs) || (compare(rhs, lhs));
            };

            auto duplicate_it = std::adjacent_find(keys_begin, keys_end, find_predicate);
            if (duplicate_it != keys_end) {
                throw std::runtime_error("Duplicate keys passed to Array_map constructor.");
            }
        }

        template<class Zip_it>
        Array_map(Zip_it begin, Zip_it end, const key_compare cmp):
            Array_map(begin, end, std::move(cmp), value_allocator_type{}) {}

        template<class Zip_it>
        Array_map(Zip_it begin, Zip_it end, const value_allocator_type& alloc):
            Array_map(begin, end, std::move(key_compare{}), alloc) {}

        template<class Zip_it>
        Array_map(Zip_it begin, Zip_it end):
            Array_map(begin, end, std::move(key_compare{}), value_allocator_type{}) {}

        ~Array_map() {
            clear();
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
            if (this == &rhs) {
                return *this;
            }

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
            if (this == &rhs) {
                return *this;
            }

            //If allocator is not propagated and allocators do not compare equal a new allocation must be made.
            bool use_new_allocation =
                !aul::is_noexcept_movable_v<A> &&
                base::operator!=(rhs);

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
            return iterator{allocation.keys, allocation.vals};
        }

        [[nodiscard]]
        const_iterator begin() const noexcept {
            return const_iterator{allocation.keys, allocation.vals};
        }

        [[nodiscard]]
        const_iterator cbegin() const noexcept {
            return const_cast<const Array_map&>(*this).begin();
        }

        [[nodiscard]]
        iterator end() noexcept {
            return iterator{allocation.keys + elem_count, allocation.vals + elem_count};
        }

        [[nodiscard]]
        const_iterator end() const noexcept {
            return const_iterator{allocation.keys + elem_count, allocation.vals + elem_count};
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
            if (this == &rhs) {
                return true;
            }

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
        V& at(const key_type& key) {
            const key_pointer pos = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);

            if (!elem_count || !pos || pos == (allocation.keys + elem_count) || *pos != key) {
                throw std::out_of_range("aul::Array_map::at() called with invalid key");
            }

            return allocation.vals[pos - allocation.keys];
        }

        [[nodiscard]]
        const V& at(const key_type& key) const {
            const key_pointer pos = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);

            if (!elem_count || !pos || pos == (allocation.keys + elem_count) || *pos != key) {
                throw std::out_of_range("aul::Array_map::at() called with invalid key");
            }

            return allocation.vals[pos - allocation.keys];
        }

        [[nodiscard]]
        V& at(key_type&& key) {
            const key_pointer pos = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);

            if (!elem_count || !pos || pos == (allocation.keys + elem_count) || *pos != key) {
                throw std::out_of_range("aul::Array_map::at() called with invalid key");
            }

            return allocation.vals[pos - allocation.keys];
        }

        [[nodiscard]]
        const V& at(const key_type&& key) const {
            const key_pointer pos = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);

            if (!elem_count || !pos || pos == (allocation.keys + elem_count) || *pos != key) {
                throw std::out_of_range("aul::Array_map::at() called with invalid key");
            }

            return allocation.vals[pos - allocation.keys];
        }



        [[nodiscard]]
        V& operator[](const key_type& key) noexcept {
            const key_pointer key_ptr = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);
            return allocation.vals[key_ptr - allocation.keys];
        }

        [[nodiscard]]
        const V& operator[](const key_type& key) const noexcept {
            const key_pointer key_ptr = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);
            return allocation.vals[key_ptr - allocation.keys];
        }

        [[nodiscard]]
        V& operator[](key_type&& key) noexcept {
            const key_pointer key_ptr = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);
            return allocation.vals[key_ptr - allocation.keys];
        }

        [[nodiscard]]
        const V& operator[](key_type&& key) const noexcept {
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

        std::pair<iterator, bool> insert(const key_type& key, value_type&& val) {
            return emplace(key, std::forward<value_type&&>(val));
        }

        std::pair<iterator, bool> insert(key_type&& key, const value_type& val) {
            return emplace(std::forward<key_type&&>(key), val);
        }

        std::pair<iterator, bool> insert(key_type&& key, value_type&& val) {
            return emplace(std::forward<key_type&&>(key), std::forward<value_type&&>(val));
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
            if (key_ptr && !empty() && (key_ptr != allocation.keys + elem_count) && compare_keys(*key_ptr, key)) {
                value_pointer ptr = allocation.vals + (key_ptr - allocation.keys);
                return std::make_pair(iterator{key_ptr, ptr}, false);
            }

            if (size() + 1 <= capacity()) {
                key_pointer keys_end = allocation.keys + elem_count;
                value_pointer vals_end = allocation.vals + elem_count;

                key_pointer new_key_ptr = key_ptr;
                value_pointer new_val_ptr = allocation.vals + (new_key_ptr - allocation.keys);

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
                return std::make_pair(iterator{new_key_ptr, new_val_ptr}, true);
            } else {
                Allocation new_allocation = allocate(grow_size(size() + 1));

                key_pointer old_key_ptr = key_ptr;

                key_pointer new_key_ptr = new_allocation.keys + (old_key_ptr - allocation.keys);
                value_pointer new_val_ptr = new_allocation.vals + (old_key_ptr - allocation.keys);

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

                value_pointer old_val_ptr = allocation.vals + (old_key_ptr - allocation.keys);
                aul::uninitialized_move(allocation.vals, old_val_ptr, new_allocation.vals, val_alloc);
                aul::uninitialized_move(old_val_ptr, allocation.vals + size(), new_val_ptr + 1, val_alloc);

                // Destroy old elements and allocation
                destroy_elements(allocation, elem_count);
                deallocate(allocation);

                allocation = std::move(new_allocation);

                ++elem_count;
                return std::make_pair(iterator{new_key_ptr, new_val_ptr}, true);
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
            if (key_ptr && !empty() && (key_ptr != allocation.keys + elem_count) && compare_keys(*key_ptr, key)) {
                value_pointer val_ptr = allocation.vals + (key_ptr - allocation.keys);
                return std::make_pair(iterator{key_ptr, val_ptr}, false);
            }

            if (size() + 1 <= capacity()) {
                key_pointer keys_end = allocation.keys + elem_count;
                value_pointer vals_end = allocation.vals + elem_count;

                key_pointer new_key_ptr = key_ptr;
                value_pointer new_val_ptr = allocation.vals + (new_key_ptr - allocation.keys);

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
                return std::make_pair(iterator{new_key_ptr, new_val_ptr}, true);
            } else {
                Allocation new_allocation = allocate(grow_size(size() + 1));

                key_pointer old_key_ptr = key_ptr;

                key_pointer new_key_ptr = new_allocation.keys + (old_key_ptr - allocation.keys);
                value_pointer new_val_ptr = new_allocation.vals + (old_key_ptr - allocation.keys);

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

                // Move objects keys and vals to new allocation
                auto val_alloc = get_allocator();
                value_pointer old_val_ptr = allocation.vals + (old_key_ptr - allocation.keys);
                aul::uninitialized_move(allocation.vals, old_val_ptr, new_allocation.vals, val_alloc);
                aul::uninitialized_move(old_val_ptr, allocation.vals + size(), new_val_ptr + 1, val_alloc);

                auto key_alloc = key_allocator_type{val_alloc};
                aul::uninitialized_move(allocation.keys, old_key_ptr, new_allocation.keys, key_alloc);
                aul::uninitialized_move(old_key_ptr, allocation.keys + size(), new_key_ptr + 1, key_alloc);

                // Destroy old elements and allocation
                destroy_elements(allocation, elem_count);
                deallocate(allocation);

                allocation = std::move(new_allocation);

                ++elem_count;
                return std::make_pair(iterator{new_key_ptr, new_val_ptr}, true);
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
            if (key_ptr && !empty() && (key_ptr != allocation.keys + elem_count) && compare_keys(*key_ptr, key)) {
                value_pointer element_ptr = allocation.vals + (key_ptr - allocation.keys);
                value_type temp{args...};
                *element_ptr = std::move(temp);
                return std::pair<iterator, bool>{iterator{key_ptr, element_ptr}, false};
            }

            if (size() + 1 <= capacity()) {
                key_pointer keys_end = allocation.keys + elem_count;
                value_pointer vals_end = allocation.vals + elem_count;

                key_pointer new_key_ptr = key_ptr;
                value_pointer new_val_ptr = allocation.vals + (new_key_ptr - allocation.keys);

                //Move elements to open up space
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
                return std::make_pair(iterator{new_key_ptr, new_val_ptr}, true);
            } else {
                Allocation new_allocation = allocate(grow_size(size() + 1));

                key_pointer old_key_ptr = key_ptr;

                key_pointer new_key_ptr = new_allocation.keys + (old_key_ptr - allocation.keys);
                value_pointer new_val_ptr = new_allocation.vals + (old_key_ptr - allocation.keys);

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

                // Move objects keys and vals to new allocation
                auto val_alloc = get_allocator();
                value_pointer old_val_ptr = allocation.vals + (old_key_ptr - allocation.keys);
                aul::uninitialized_move(allocation.vals, old_val_ptr, new_allocation.vals, val_alloc);
                aul::uninitialized_move(old_val_ptr, allocation.vals + size(), new_val_ptr + 1, val_alloc);

                auto key_alloc = key_allocator_type{val_alloc};
                aul::uninitialized_move(allocation.keys, old_key_ptr, new_allocation.keys, key_alloc);
                aul::uninitialized_move(old_key_ptr, allocation.keys + size(), new_key_ptr + 1, key_alloc);

                // Destroy old elements and allocation
                destroy_elements(allocation, elem_count);
                deallocate(allocation);

                allocation = std::move(new_allocation);

                ++elem_count;
                return std::pair<iterator, bool>{iterator{new_key_ptr, new_val_ptr}, true};
            }
        }

        ///
        /// Attempts to construct a value object in place
        ///
        /// If an element is associated with the specified key, constructs a
        /// temporary which is move assigned to the previously mapped element.
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
                value_pointer element_ptr = allocation.vals + (key_ptr - allocation.keys);
                value_type temp{args...};
                *element_ptr = std::move(temp);
                return std::pair<iterator, bool>{iterator{key_ptr, element_ptr}, false};
            }

            if (size() + 1 <= capacity()) {
                //Current allocation suffices

                key_pointer keys_end = allocation.keys + elem_count;
                value_pointer vals_end = allocation.vals + elem_count;

                key_pointer new_key_ptr = key_ptr;
                value_pointer new_val_ptr = allocation.vals + (new_key_ptr - allocation.keys);

                //Move elements to new allocation
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
                return std::make_pair(iterator{new_key_ptr, new_val_ptr}, true);
            } else {
                //New allocation is necessary

                Allocation new_allocation = allocate(grow_size(size() + 1));

                key_pointer old_key_ptr = key_ptr;

                key_pointer new_key_ptr = new_allocation.keys + (old_key_ptr - allocation.keys);
                value_pointer new_val_ptr = new_allocation.vals + (old_key_ptr - allocation.keys);

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

                // Move objects keys and vals to new allocation

                auto val_alloc = get_allocator();
                auto key_alloc = key_allocator_type{val_alloc};
                aul::uninitialized_move(allocation.keys, old_key_ptr, new_allocation.keys, key_alloc);
                aul::uninitialized_move(old_key_ptr, allocation.keys + size(), new_key_ptr + 1, key_alloc);

                value_pointer old_val_ptr = allocation.vals + (old_key_ptr - allocation.keys);
                aul::uninitialized_move(allocation.vals, old_val_ptr, new_allocation.vals, val_alloc);
                aul::uninitialized_move(old_val_ptr, allocation.vals + size(), new_val_ptr + 1, val_alloc);

                // Destroy old elements and allocation
                destroy_elements(allocation, elem_count);
                deallocate(allocation);

                allocation = std::move(new_allocation);

                ++elem_count;
                return std::make_pair(iterator{new_key_ptr, new_val_ptr}, true);
            }

        }

        //=================================================
        // Erasure methods
        //=================================================

        ///
        /// \param pos Valid iterator to element
        /// \return Iterator to element which replaced the removed element
        iterator erase(iterator pos) noexcept {
            key_pointer key_ptr = get<0>(pos);
            value_pointer val_ptr = get<1>(pos);

            std::move(key_ptr + 1, allocation.keys + elem_count, key_ptr);
            std::move(val_ptr + 1, allocation.vals + elem_count, val_ptr);

            destroy_key(key_ptr + elem_count - 1);
            destroy_val(val_ptr + elem_count - 1);

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
            key_pointer key_ptr = {aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator)};

            if (key_ptr && (key_ptr != allocation.keys + elem_count) && compare_keys(*key_ptr, key)) {
                return iterator{key_ptr, allocation.vals + (key_ptr - allocation.keys)};
            } else {
                return end();
            }
        }

        ///
        /// \param key Key to search for
        /// \return Iterator to element. end() if not found
        [[nodiscard]]
        const_iterator find(const key_type& key) const noexcept {
            key_pointer key_ptr = {aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator)};

            if (key_ptr && (key_ptr != allocation.keys + elem_count) && compare_keys(*key_ptr, key)) {
                return const_iterator{key_ptr, allocation.vals + (key_ptr - allocation.keys)};
            } else {
                return cend();
            }
        }

        template<class K2>
        [[nodiscard]]
        iterator find(const K2& key) noexcept {
            key_pointer key_ptr = {aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator)};

            if (key_ptr && (key_ptr != allocation.keys + elem_count) && compare_keys(*key_ptr, key)) {
                return iterator{key_ptr, allocation.vals + (key_ptr - allocation.keys)};
            } else {
                return end();
            }
        }

        template<class K2>
        [[nodiscard]]
        const_iterator find(const K2& key) const noexcept {
            key_pointer key_ptr = {aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator)};

            if (key_ptr && (key_ptr != allocation.keys + elem_count) && compare_keys(*key_ptr, key)) {
                return const_iterator{key_ptr, allocation.vals + (key_ptr - allocation.keys)};
            } else {
                return cend();
            }
        }

        ///
        /// \param key Key to search for
        /// \returns True if key maps to an element
        [[nodiscard]]
        bool contains(const key_type& key) const noexcept {
            key_pointer ptr = aul::binary_search(allocation.keys, allocation.keys + elem_count, key, comparator);
            return (ptr && size() && compare_keys(*ptr, key));
        }

        [[nodiscard]]
        V& get_or_default(const key_type& key, V& def) noexcept {
            auto it = find(key);
            if (it == end()) {
                return def;
            } else {
                return std::get<1>(*it);
            }
        }

        [[nodiscard]]
        const V& get_or_default(const key_type& key, const V& def) noexcept {
            auto it = find(key);
            if (it == end()) {
                return def;
            } else {
                return std::get<1>(*it);
            }
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
        A get_allocator() const {
            return base::get_allocator();
        }

        //=================================================
        // Size/capacity methods
        //=================================================

        ///
        ///
        /// \return Number of elements held
        [[nodiscard]]
        size_type size() const noexcept {
            return elem_count;
        }

        ///
        ///
        /// \return Maximum allocation size
        [[nodiscard]]
        size_type max_size() const noexcept {
            constexpr size_type a = std::numeric_limits<size_type>::max();
            size_type b = std::allocator_traits<A>::max_size(get_allocator());
            return std::min(a, b);
        }

        ///
        ///
        /// \return Size of allocation
        [[nodiscard]]
        size_type capacity() const noexcept {
            return allocation.capacity;
        }

        ///
        ///
        /// \return True if no elements exist
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
        /// Provides the strong exception guarantee
        ///
        /// \param n Number to increase capacity to
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
        ///
        /// \return Binary predicate object that compares keys
        [[nodiscard]]
        key_compare key_comp() const noexcept {
            return comparator;
        }

        ///
        ///
        /// \return Binary predicate object that compares element's ordering
        [[nodiscard]]
        value_compare value_comp() const noexcept {
            return value_compare{};
        }

        /*
        ///
        /// \return Pointer to internal value array
        ///
        [[nodiscard]]
        pointer data() const noexcept {
            return allocation.vals;
        }
        */

        ///
        ///
        /// \return Pointer to value array
        [[nodiscard]]
        value_pointer value_data() const noexcept {
            return allocation.vals;
        }

        ///
        ///
        /// \return Pointer to key array
        [[nodiscard]]
        key_pointer key_data() const noexcept {
            return allocation.keys;
        }

        ///
        ///
        /// \return Span over key array
        [[nodiscard]]
        aul::Span<const key_type> keys() const noexcept {
            return {allocation.keys, elem_count};
        }

        ///
        ///
        /// \return Span over value array
        [[nodiscard]]
        aul::Span<const value_type> values() const noexcept {
            return {allocation.vals, elem_count};
        }

        ///
        ///
        /// \return Span over value array
        [[nodiscard]]
        aul::Span<value_type> values() noexcept {
            return {allocation.vals, elem_count};
        }

        /*
        // Just use aul::Span for consistency across version. Could cause
        // problems otherwise.
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
        */

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
        static Allocation allocate(value_allocator_type& allocator, const size_type n) {
            Allocation ret{};

            try {
                ret.vals = std::allocator_traits<value_allocator_type>::allocate(allocator, n);
                auto key_alloc = key_allocator_type{allocator};
                ret.keys = std::allocator_traits<key_allocator_type>::allocate(key_alloc, n);
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
        static Allocation allocate(value_allocator_type& allocator, const size_type n, const Allocation& hint) {
            Allocation ret{};

            try {
                ret.vals = std::allocator_traits<value_allocator_type>::allocate(allocator, n, hint.vals);
                key_allocator_type key_alloc{allocator};
                ret.keys = std::allocator_traits<key_allocator_type>::allocate(key_alloc, n, hint.keys);
            } catch (...) {
                deallocate(allocator, ret);
                throw;
            }

            ret.capacity = n;

            return ret;
        }

        void deallocate(Allocation& a) noexcept {
            auto val_alloc = get_allocator();
            auto key_alloc = key_allocator_type{};
            std::allocator_traits<value_allocator_type>::deallocate(val_alloc, a.vals, a.capacity);
            std::allocator_traits<key_allocator_type>::deallocate(key_alloc, a.keys, a.capacity);

            a = Allocation{};
        }

        ///
        /// \param alloc Allocation to free. Empty after successful completion
        ///
        static void deallocate(value_allocator_type& val_alloc, Allocation& allocation) noexcept {
            auto key_alloc = key_allocator_type{val_alloc};
            std::allocator_traits<value_allocator_type>::deallocate(val_alloc, allocation.vals, allocation.capacity);
            std::allocator_traits<key_allocator_type>::deallocate(key_alloc, allocation.keys, allocation.capacity);

            allocation = Allocation{};
        }

        //=================================================
        // Construction/Destruction helper methods
        //=================================================s

        template<class...Args>
        void construct_val(value_pointer ptr, Args&&...args) {
            auto val_alloc = get_allocator();
            std::allocator_traits<value_allocator_type>::construct(val_alloc, ptr, std::forward<Args>(args)...);
        }

        template<class...Args>
        void construct_key(key_pointer ptr, Args&&...args) {
            auto key_alloc = key_allocator_type{get_allocator()};
            std::allocator_traits<key_allocator_type>::construct(key_alloc, ptr, std::forward<Args>(args)...);
        }

        void destroy_val(value_pointer ptr) {
            value_allocator_type val_alloc{get_allocator()};
            std::allocator_traits<value_allocator_type>::destroy(val_alloc, ptr);
        }

        void destroy_key(key_pointer ptr) {
            key_allocator_type alloc{get_allocator()};
            std::allocator_traits<key_allocator_type>::destroy(alloc, ptr);
        }

        ///
        /// \param source Allocation move elements from
        /// \param dest Allocation to move elements to
        /// \param n Number of elements to copy
        void move_elements(Allocation& source, Allocation& dest, size_type n) noexcept {
            auto val_alloc = get_allocator();
            auto key_alloc = key_allocator_type {val_alloc};

            aul::uninitialized_move_n(source.vals, n, dest.vals, val_alloc);
            aul::uninitialized_move_n(source.keys, n, dest.keys, key_alloc);
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

        ///
        /// Destroy elements in allocation using current object's allocator
        ///
        /// \param source Allocation containing elements
        /// \param n Number of elements to destroy
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

        ///
        /// Compares keys for equality using internal comparator object
        ///
        /// \param a First key
        /// \param b Second key
        /// \return True if neither key compares less than the other
        bool compare_keys(const key_type& a, const key_type& b) const {
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
            //Values at lower addresses map to keys which are less than the
            //keys for elements at higher addresses
            return aul::get<1>(x) < aul::get<1>(y);
        }

    };

    template<typename K, typename T, typename C, typename A>
    class Array_map<K, T, C, A>::Allocation {
    public:

        //=================================================
        // Instance members
        //=================================================

        value_pointer vals = nullptr;
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
            if (this == &alloc) {
                return *this;
            }

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

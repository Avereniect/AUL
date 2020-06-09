#ifndef AUL_SLOT_MAP_HPP
#define AUL_SLOT_MAP_HPP

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

#include "../Versioned_type.hpp"
#include "../memory/Memory.hpp"
#include "../Algorithms.hpp"
#include "Random_access_iterator.hpp"

#include <algorithm>
#include <initializer_list>
#include <climits>
#include <memory>
#include <numeric>
#include <string>
#include <type_traits>
#include <utility>

namespace aul {

    /// Slot_map
    ///
    /// A vector like associative container offering constant time look-up,
    /// insertion, and deletion. 
    ///
    /// Much like a vector, elements are stored in a contiguous array and can be
    /// be referenced via their index. 
    ///
    /// Additionally, each element is mapped to a unique key object from its
    /// construction to its destruction. This key can be retrieved via the
    /// get_key() method. A key becomes invalid once the element that it was
    /// mapped to no longer exists within the container. The validity of a key
    /// can be checked via the contains() method. 
    ///
    /// Algorithms such as std::sort and std::reverse may be applied to the
    /// contents of this container however all keys are liable to lose their
    /// associations. Keys will still map to valid elements but the resulting
    /// mappings are not predictable. 
    ///
    /// A default-constructed value of key_type will is very unlikely to map to
    /// any object and thus can effectively be used as a null key. 
    ///
    /// \tparam T Element type
    /// \tparam A Allocator type
    template<class T, class A = std::allocator<T>>
    class Slot_map {

        //=================================================
        // Helper classes
        //=================================================


        class Allocation;
        class Metadata;

        class Key {
            friend class Slot_map;
            using key_primitive = typename std::allocator_traits<A>::size_type;

            Key() = default;
            Key(const key_primitive i, const key_primitive v):
                index(i),
                version(v) {}

            key_primitive index = std::numeric_limits<key_primitive>::max();
            key_primitive version = std::numeric_limits<key_primitive>::max();
        };

    public:

        //=================================================
        // Type aliases
        //=================================================

        using allocator_type = A;

        using size_type = typename std::allocator_traits<A>::size_type;
        using difference_type = typename std::allocator_traits<A>::difference_type;

        using pointer = typename std::allocator_traits<allocator_type>::pointer;
        using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;

        using value_type = T;
        using key_type = Key;

        using reference = T&;
        using const_reference = const T&;

        using iterator = Random_access_iterator<typename aul::Allocator_types<A>, false>;
        using const_iterator = Random_access_iterator<typename aul::Allocator_types<A>, true>;

        using reverse_iterator = typename std::reverse_iterator<iterator>;
        using const_reverse_iterator = typename std::reverse_iterator<const_iterator>;

    private:

        using allocator_traits = std::allocator_traits<allocator_type>;

        using md_allocator_type = typename std::allocator_traits<A>::template rebind_alloc<Metadata>;
        using md_allocator_traits = std::allocator_traits<md_allocator_type>;
        using md_pointer = typename md_allocator_traits::pointer;

    public:

        //=================================================
        // -ctors
        //=================================================

        ///
        /// Default constructor
        ///
        Slot_map() noexcept(noexcept(allocator_type{})) = default;

        ///
        /// \param alloc Allocator to copy-construct internal allocators from
        ///
        explicit Slot_map(const allocator_type& alloc) noexcept:
            allocator(alloc) {}

        ///
        /// \param n     Number of copies made of val
        /// \param val   Source for copy-construction of elements
        /// \param alloc Source for copy-construction of internal allocator
        ///
        Slot_map(const size_type n, const T& val, const allocator_type& alloc = {}) :
            allocator(alloc),
            allocation(allocate(n)),
            elem_count(n) {

            aul::uninitialized_fill(allocation.elements, allocation.elements + n, val, allocator);
            generate_default_metadata(n);
        }

        ///
        /// \param n     Number of elements to default construct
        /// \param alloc Source for copy-construction of internal allocator
        ///
        explicit Slot_map(const size_type n, const allocator_type& alloc = {}) :
            allocator(alloc),
            allocation(allocate(n)),
            elem_count(n) {

            auto index_allocator = md_allocator_type{allocator};

            aul::default_construct(allocation.elements, allocation.elements + n, allocator);
            generate_default_metadata(n);
        }

        ///
        /// \param begin Iterator to first element in source range
        /// \param end   Iterator to one past the last element in source range
        /// \param alloc Source for copy-construction of internal allocator
        ///
        template<class InputIter>
        Slot_map(InputIter begin, InputIter end, const allocator_type& alloc = {}) :
            allocator(alloc),
            allocation(allocate(end - begin)),
            elem_count(end - begin) {

            auto index_allocator = md_allocator_type{allocator};

            aul::uninitialized_copy(begin, end, allocation.elements, allocator);
            generate_default_metadata(end - begin);
        }

        /// 
        /// \param list  Source list for copy construction of elements
        /// \param alloc Source for copy constructing internal allocators
        /// 
        Slot_map(const std::initializer_list<T>& list, allocator_type allocator = {}) :
            Slot_map(list.begin(), list.end(), allocator) {}

        ///
        /// \param right Source object
        ///
        Slot_map(Slot_map&& right) noexcept:
            allocator(std::move(right.allocator)),
            allocation(std::move(right.allocation)),
            elem_count(std::move(right.elem_count)),
            free_anchor(std::move(right.free_anchor)) {

            right.elem_count = 0;
            right.free_anchor = nullptr;
        }

        ///
        /// \param right Source object
        /// \param alloc Source for copy-construction of internal allocator
        ///
        Slot_map(Slot_map&& right, allocator_type alloc) noexcept:
            allocator(alloc),
            allocation((alloc == right.get_allocator()) ? std::move(right.allocation) : allocate(right.capacity())),
            elem_count(right.elem_count),
            free_anchor(allocation.metadata + (right.free_anchor - right.allocation.metadata)) {

            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy constructable.");

            if (right.allocator != alloc) {
                aul::uninitialized_move_n(right.allocation.elements, elem_count, allocation.elements, allocator);
            }

            right.elem_count = 0;
            right.free_anchor = nullptr;
        }

        ///
        /// \param src Source object
        ///
        Slot_map(const Slot_map& src) :
            allocator(allocator_traits::select_on_container_copy_construction(src.allocator)),
            allocation(allocate(src.allocation.capacity)),
            elem_count(src.elem_count),
            free_anchor(allocation.metadata + (src.free_anchor - src.allocation.metadata)) {

            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy constructable.");

            auto md_allocator = md_allocator_type{allocator};

            aul::uninitialized_copy(src.allocation.elements, src.allocation.elements + src.elem_count, allocation.elements, allocator);
            aul::uninitialized_copy(src.allocation.metadata, src.allocation.metadata + src.elem_count, allocation.metadata, md_allocator);
        }

        ///
        /// \param src Source object
        /// \param alloc Source for copy-construction of internal allocator
        Slot_map(const Slot_map& src, const allocator_type& alloc) :
            allocator(alloc),
            allocation(allocate(src.allocation.capacity)),
            elem_count(src.elem_count),
            free_anchor(allocation.metadata + (src.free_anchor - src.allocation.metadata)) {

            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy constructable.");

            auto md_allocator = md_allocator_type{allocator};

            aul::uninitialized_copy(src.allocation.elements, src.allocation.elements + elem_count, allocation.elements, allocator);
            aul::uninitialized_copy(src.allocation.metadata, src.allocation.metadata + elem_count, allocation.metadata, md_allocator);
        }

        ///
        /// Destructor
        ///
        ~Slot_map() {
            aul::destroy(allocation.elements, allocation.elements + size(), allocator);
            aul::destroy(allocation.metadata, allocation.metadata + capacity(), allocator);
            deallocate(allocation);
        }

        //=================================================
        // Modifier methods
        //=================================================

        ///
        /// Destructs current contents. Reduces capacity to 0.
        ///
        void clear() noexcept {
            if (allocation.capacity) {
                auto md_allocator = md_allocator_type{allocator};

                aul::destroy(allocation.elements, allocation.elements + elem_count, allocator);
                aul::destroy(allocation.metadata, allocation.metadata + allocation.capacity, md_allocator);
            }

            deallocate(allocation);

            elem_count = 0;
            free_anchor = nullptr;
        }

        /// Replaces the contents of the current object those of src. Also swaps
        /// allocators if necessary.
        ///
        /// \param src Target object to swap with
        ///
        void swap(Slot_map& src) noexcept(
            allocator_traits::propagate_on_container_swap::value ||
            allocator_traits::is_always_equal::value) {

            if constexpr (allocator_traits::propagate_on_container_swap::value) {
                std::swap(allocator, src.allocator);
            }

            std::swap(allocation, src.allocation);
            std::swap(elem_count, src.elem_count);
            std::swap(free_anchor, src.free_anchor);
        }

        ///
        /// \param l Left map to swap
        /// \param r Right map to swap
        ///
        friend void swap(Slot_map& l, Slot_map& r) {
            l.swap();
        }

        //=================================================
        // Assignment methods & operators
        //=================================================

        /// Replaces current content with those in list. New contents are copy-
        /// constructed from originals.
        ///
        ///
        /// \param list Source list for elements
        ///
        void assign(std::initializer_list<T> list) {
            assign(list.begin(), list.end());
        }

        /// Replaces current contents with those in the range specified by
        /// [begin, end). New contents are copy-constructed from originals.
        ///
        /// \tparam Iter  Iterator type
        /// \param begin Iterator to first element in source range
        /// \param end   Iterator to on past last element in source range
        ///
        template<typename Iter>
        void assign(Iter begin, Iter end) {
            size_type new_size = end - begin;
            if (new_size < capacity()) {

            }

            free_anchor = nullptr;
            elem_count = allocation.capacity;

            auto md_allocator = md_allocator_type{allocator};

            aul::uninitialized_copy(begin, end, allocation.elements, allocator);
            aul::uninitialized_iota(allocation.metadata, allocation.metadata + elem_count, 0, md_allocator);
        }

        /// Replaces current contents with n copies of val.
        ///
        /// \param n   Number of copies made of val
        /// \param val Source for copy-construction of new elements
        ///
        void assign(const size_type n, const T& val) {
            clear();
            reserve(n);

            free_anchor = nullptr;
            elem_count = n;

            auto md_allocator = md_allocator_type{allocator};

            aul::uninitialized_fill(allocation.elements, allocation.elements + n, val, allocator);
            aul::uninitialized_iota(allocation.metadata, allocation.metadata + n, 0, md_allocator);
        }

        /// Copy assignment operator
        ///
        /// \param  src 
        /// \return Current object
        ///
        Slot_map& operator=(const Slot_map& src) {
            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy-constructible.");

            if (this == &src) {
                return *this;
            }

            clear();

            if constexpr (allocator_traits::propagate_on_container_copy_assignment::value) {
                allocator = src.allocator;
            }
            allocation = allocate(src.allocation.capacity);
            elem_count = src.elem_count;
            free_anchor = allocation.metadata + (src.free_anchor - src.allocation.metadata);

            auto md_allocator = md_allocator_type{allocator};

            aul::uninitialized_copy(src.allocation.elements, src.allocation.elements + elem_count, allocation.elements, allocator);
            aul::uninitialized_copy(src.allocation.metadata, src.allocation.metadata + elem_count, allocation.metadata, md_allocator);

            return *this;
        }

        /// Move assignment operator
        /// \param src Target object to move from
        /// \return Current object
        ///
        Slot_map& operator=(Slot_map&& src) noexcept {
            if (this == &src) {
                return *this;
            }

            clear();

            if constexpr (allocator_traits::propagate_on_container_move_assignment::value) {
                allocator = std::move(src.allocator);
            }

            allocation = std::move(allocation);
            elem_count = std::move(elem_count);
            free_anchor = std::move(free_anchor);

            src.elem_count = 0;
            src.free_anchor = nullptr;

            return *this;
        }

        /// Replaces current contents with those of list.
        ///
        /// \param list Source for replace
        /// \return  Current object
        ///
        Slot_map& operator=(const std::initializer_list<T> list) {
            assign(list);
            return *this;
        }

        //=================================================
        // Access methods
        //=================================================

        ///
        /// \return Reference to first element. Undefined if empty
        ///
        [[nodiscard]]
        T& front() {
            return allocation.elements[0];
        }

        ///
        /// \return Const reference to first element. Undefined if empty
        ///
        [[nodiscard]]
        const T& front() const {
            return allocation.elements[0];
        }

        ///
        /// \return Reference to last element. Undefined if empty
        ///
        [[nodiscard]]
        T& back() {
            return allocation.elements[elem_count - 1];
        }

        ///
        /// \return Const reference to last element. Undefined if empty
        ///
        [[nodiscard]]
        const T& back() const {
            return allocation.elements[elem_count - 1];
        }

        /// \param x Index of desired element
        /// \return  Reference to element at index x
        ///
        [[nodiscard]]
        T& at(const size_type x) {
            if (size() <= x) {
                throw std::out_of_range("Index out of bounds.");
            }

            return allocation.elements[x];
        }

        /// \param x Index of desired element
        /// \return  Reference to element at index x
        ///
        [[nodiscard]]
        const T& at(const size_type x) const {
            if (size() <= x) {
                throw std::out_of_range("Index out of bounds.");
            }

            return allocation.elements[x];
        }

        /// \param x Key mapped to desired element
        /// \return  Reference to element mapped to key x
        ///
        [[nodiscard]]
        T& at(const key_type k) {
            if (size() <= k.index) {
                throw std::out_of_range("Index out of bounds.");
            }

            return operator[](k);
        }

        /// \param x Key mapped to desired element
        /// \return  Reference to element mapped to key x
        ///
        [[nodiscard]]
        const T& at(const key_type k) const {
            if (size() <= k.index) {
                throw std::out_of_range("Index out of bounds.");
            }

            return operator[](k);
        }

        //=====================================================================
        // Access operators
        //=====================================================================

        /// \param x Index of desired element
        /// \return  Reference to element at index x
        ///
        [[nodiscard]]
        T& operator[](const size_type x) {
            return allocation.elements[x];
        }

        /// \param x Index of desired element
        /// \return  Reference to element at index x
        ///
        [[nodiscard]]
        const T& operator[](const size_type x) const {
            return allocation.elements[x];
        }

        ///
        /// Undefined behavior if key is not valid
        ///
        /// \param x Key mapped to desired element
        /// \return  Reference to element mapped to key x
        ///
        [[nodiscard]]
        T& operator[](const key_type key) {
            size_type index = allocation.metadata[key.index].anchor.data();
            return allocation.elements[index];
        }

        ///
        /// Undefined behavior if key is not valid
        ///
        /// \param x Key mapped to desired element
        /// \return  Reference to element mapped to key x
        ///
        [[nodiscard]]
        const T& operator[](const key_type key) const {
            size_type index = allocation.metadata[key.index].anchor.data();
            return allocation.elements[index];
        }

        //=================================================
        // Element addition
        //=================================================

        /// Constructs an object from a set of parameters at a specified place
        /// Is stable.
        ///
        /// \tparam Args Argument types for constructor call
        /// \param args  Constructor arguments for construction of new element
        /// \return      Reference to newly constructed object
        ///
        template<class... Args>
        key_type emplace(Args&& ... args);

        ///
        /// \param count Number of elements to be inserted
        /// \param value Object from which elements are inserted
        ///
        iterator insert(const size_type count, const T& value);

        ///
        /// \tparam Input_iter Type of iterator specifying source range
        /// \param begin Iterator to begining of source range
        /// \param end   Iterator to end of source range
        ///
        template<class Input_iter>
        iterator insert(Input_iter begin, Input_iter end);

        ///
        /// \param list Initiializer list to copy-construct elements from
        ///
        iterator insert(std::initializer_list<T> list) {
            insert(list.begin(), list.end());
        }

        ///
        /// Constructs an object from a set of parameters at the last position
        /// in the container.
        ///
        /// \tparam Args Argument types for constructor call
        /// \param args  Constructor arguments for construction of new element
        /// \return      Reference to newly constructed object
        ///
        template<class... Args>
        key_type emplace_back(Args&& ... args);

        ///
        /// \param val Object to copy-construct from
        ///
        key_type push_back(const T& val) {
            return emplace_back(val);
        }

        ///
        /// \param val Object to move-construct from
        ///
        key_type push_back(T&& val) {
            return emplace_back(std::forward<T&&>(val));
        }

        //=================================================
        // Element removal
        //=================================================

        /// Removes the last element in the container. undefined if container
        /// is empty.
        ///
        void pop_back() {
            --elem_count;
            destroy_element(allocation.elements + elem_count);
        }


        ///
        /// \param key Valid key mapping to an element
        ///
        void erase(const key_type key) {
            const size_type pos = allocation.metadata[key.index].anchor;
            const pointer ptr = allocation.elements + pos;
            swap_elements(ptr, std::addressof(end()[-1]));
            pop_back();
        }

        ///
        /// \param it Valid iterator to element to erase
        ///
        void erase(const_iterator it) {
            const pointer l = const_cast<pointer>(it.operator->());
            const pointer r = allocation.elements + elem_count - 1;
            swap_elements(l, r);
            pop_back();
        }

        //=================================================
        // Iterator methods
        //=================================================

        iterator begin() noexcept {
            return iterator(allocation.elements);
        }

        const_iterator begin() const {
            return const_iterator(allocation.elements);
        }

        const_iterator cbegin() const {
            return const_cast<const Slot_map&>(*this).begin();
        }

        iterator end() {
            iterator it(allocation.elements ? allocation.elements + elem_count : nullptr);
            return it;
        }

        const_iterator end() const {
            const_iterator it(allocation.elements ? allocation.elements + elem_count : nullptr);
            return it;
        }

        const_iterator cend() const {
            return const_cast<const Slot_map&>(*this).end();
        }


        reverse_iterator rbegin() {
            return reverse_iterator(end());
        }

        const_reverse_iterator rbegin() const {
            return const_reverse_iterator(end());
        }

        const_reverse_iterator crbegin() const {
            return const_cast<const Slot_map&>(*this).rbegin();
        }

        reverse_iterator rend() {
            return reverse_iterator(begin());
        }

        const_reverse_iterator rend() const {
            return const_reverse_iterator(begin());
        }

        const_reverse_iterator crend() const {
            return const_cast<const Slot_map&>(*this).rend();
        }

        //=================================================
        // Size & capacity methods
        //=================================================

        ///
        /// \return True if container has no elements
        ///
        [[nodiscard]]
        bool empty() const {
            return elem_count == 0;
        }

        ///
        /// \return Allocation capacity
        ///
        [[nodiscard]]
        size_type capacity() const {
            return allocation.capacity;
        }

        ///
        /// \return Element count
        ///
        [[nodiscard]]
        size_type size() const {
            return elem_count;
        }

        ///
        /// \return Maximum capacity container may reach.
        ///
        [[nodiscard]]
        size_type max_size() const {
            constexpr size_type size_type_max = std::numeric_limits<difference_type>::max();
            const size_type element_max = sizeof(value_type) * allocator_traits::max_size(allocator);

            const size_type memory_max = element_max / (sizeof(value_type) + sizeof(Metadata));

            return std::min(size_type_max, memory_max);
        }

        ///
        /// Allocates at least enough memory to store n elements. Current
        /// implementation allocates exactly enough memory for n elements.
        ///
        /// \param n Number of elements to allocate memory for
        ///
        void reserve(const size_type n) {
            if (n <= capacity()) {
                return;
            }

            if (max_size() < n) {
                throw std::length_error("Slot_map grew beyond max size");
            }

            //Make new allocation
            Allocation new_allocation = allocate(n);
            
            //Temporary allocator for indices
            auto md_allocator = md_allocator_type{allocator};

            //Move contents of array if location of arrays has changed
            {
                if (new_allocation.metadata != allocation.metadata) {
                    aul::uninitialized_move(allocation.metadata, allocation.metadata + capacity(), new_allocation.metadata, md_allocator);
                }

                if (new_allocation.elements != allocation.elements) {
                    aul::uninitialized_move(allocation.elements, allocation.elements + elem_count, new_allocation.elements, allocator);
                }
            }

            //Create new empty indices and add to free index list
            {
                //Pointer to first of new indices
                md_pointer new_metadata = new_allocation.metadata + allocation.capacity;

                //Number of new indices to be create minus one
                const size_type new_metadata_count = (n - capacity()) - 1;

                //Construct new indices except for last
                size_type data = (new_metadata - new_allocation.metadata) + 1;
                for (size_type i = 0; i < new_metadata_count; ++i, ++data) {
                    md_allocator_traits::construct(md_allocator, new_metadata + i, data, 1);
                }

                //Pointer to last newly created index
                const md_pointer new_metadata_last = new_metadata + new_metadata_count;

                //Contents of new last new index
                data = (free_anchor) ? (free_anchor - allocation.metadata) : (new_metadata_last - new_allocation.metadata);

                //Construct new last index
                md_allocator_traits::construct(md_allocator, new_metadata_last, data, 1);

                free_anchor = new_metadata;
            }

            deallocate(allocation);
            allocation = std::move(new_allocation);
        }

        /// Resizes the container to contain exact n many elements. If n is 
        /// less than the current size, the excess objects are deleted. If n is
        /// greater than the current size default constructed elements are
        /// inserted to the end of the container.
        ///
        void resize(const size_type n) {
            resize(n, value_type{});
        }

        /// Resizes the container to contain exact n many elements. If n is 
        /// less than the current size, the excess objects are deleted. If n is
        /// greater than the current size copies of val are inserted to the end
        /// of the container.
        ///
        void resize(const size_type n, const T& val) {
            if (size() < n) {
                for (iterator it = begin() + n; it != end() + n; ++it) {
                    erase(it);
                }

                return;
            }

            if (n < size()) {
                reserve(n);

                for (size_type i = 0; i != n; ++i) {
                    emplace_back(val);
                }

                return;
            }
        }

        //=================================================
        // Comparison operators
        //=================================================

        [[nodiscard]]
        friend bool operator==(const Slot_map& lhs, const Slot_map& rhs) {
            return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
        }

        [[nodiscard]]
        friend bool operator!=(const Slot_map& lhs, const Slot_map& rhs) {
            return !operator==(lhs, rhs);
        }

        [[nodiscard]]
        friend bool operator<(const Slot_map& lhs, const Slot_map& rhs) {
            return aul::less_than(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
        }

        [[nodiscard]]
        friend bool operator>(const Slot_map& lhs, const Slot_map& rhs) {
            return aul::greater_than(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
        }

        [[nodiscard]]
        friend bool operator<=(const Slot_map& lhs, const Slot_map& rhs) {
            return aul::less_than_or_equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
        }

        [[nodiscard]]
        friend bool operator>=(const Slot_map& lhs, const Slot_map& rhs) {
            return aul::greater_than_or_equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
        }

        //=================================================
        // Misc. methods
        //=================================================

        /// \param it Iterator to element
        /// \return   key corresponding to element pointed to be it
        ///
        [[nodiscard]]
        key_type get_key(const_iterator it) {
            const_pointer p = it.operator->();

            const size_type x = allocation.metadata[p - allocation.elements].anchor_index;
            const size_type y = allocation.metadata[p - allocation.elements].anchor.version();

            return key_type{x, y};
        }

        /// \param x Key to be checked
        /// \return  Returns true if the key maps to a valid element
        ///
        [[nodiscard]]
        bool contains(key_type x) {
            return
                x.version == allocation.metadata[x.index].anchor.version() &&
                x.index <= size();
        }

        ///
        /// \return Copy of internal allocator
        ///
        [[nodiscard]]
        allocator_type get_allocator() const {
            return allocator;
        }

        ///
        /// \return Pointer to array containing elements
        ///
        [[nodiscard]]
        pointer data() {
            return allocation.elements;
        }

        ///
        /// \return Pointer to array containing elements
        ///
        [[nodiscard]]
        const_pointer data() const {
            return allocation.elements;
        }

    private:

        //=================================================
        // Instance members
        //=================================================

        allocator_type allocator{};

        Allocation allocation{};

        size_type elem_count = 0;

        md_pointer free_anchor = nullptr;

        //=================================================
        // Misc. helper methods
        //=================================================

        /// Increases the capacity of the container via doubling to at least n
        ///
        /// \param n Number of elements
        void grow(const size_type n) {
            const size_type double_size = (max_size() / 2) < size() ? max_size() : 2 * capacity();
            reserve(std::max(n, double_size));
        }

        /// Returns the index associated with the element pointed to by ptr
        ///
        /// \param Pointer to element in element array
        [[nodiscard]]
        Metadata& metadata_of(const_pointer ptr) const noexcept {
            return allocation.metadata[allocation.metadata[ptr - allocation.elements].anchor];
        }

        //=================================================
        // Anchor index helper methods
        //=================================================

        /// Takes free index and populates it, making it point to the position
        /// indicated by pos.
        ///
        /// \pre An index that is free must exist. i.e. free_index != nullptr
        /// \param pos Index of element to be held yb metadata
        /// \return Pointer to index that has been consumed.
        ///
        md_pointer consume_anchor(const size_type pos) noexcept {
            const md_pointer free_ptr = free_anchor;

            //If free list terminates, assign nullptr to free_index, otherwise
            //assign the next node.
            if (free_ptr->anchor_index == (free_ptr - allocation.metadata)) {
                free_anchor = nullptr;
            } else {
                free_anchor = allocation.metadata + (free_ptr->anchor_index);
            }

            free_ptr->anchor_index = pos;

            return free_ptr;
        }

        /// Frees index pointed to by ptr and pushes it onto list of free
        /// indices. Increments index version.
        ///
        void release_anchor(const md_pointer ptr) noexcept {
            if (free_anchor) {
                *ptr = free_anchor - allocation.metadata;
            } else {
                *ptr = ptr - allocation.metadata;
            }
            free_anchor = ptr;
        }

        //=================================================
        // Element helper methods
        //=================================================

        void generate_default_metadata(const size_type n) {
            auto md_allocator = md_allocator_type{allocator};
            for (size_type i = 0; i < n; ++i) {
                md_pointer ptr = allocation.metadata + i;
                md_allocator_traits::construct(md_allocator, ptr, i);
            }

            //Handle linked list termination

        }

        /// Destroys the element pointed to by p through the allocator and
        /// clears the associated inde value.
        /// \param p Pointer to element to be destroyed.
        ///
        void destroy_element(pointer p) {
            //TODO: Implement
        }

        /// Constructs an element at the specified position along with the
        /// corresponding erase value and index
        ///
        /// \pre mem.free_head points to an already constructed index object
        /// \tparam     Args args type
        /// \param pos  Address to construct element at
        /// \param args Parameters to element constructor
        template<class...Args>
        void construct_element(pointer pos, Args&& ... args) {
            md_pointer index = free_anchor;
            consume_anchor(pos - allocation.elements);
            allocator_traits::construct(allocator, pos, std::forward<Args>(args)...);
        }

        /// Move constructs an element within the container from its current
        /// position to dest, and updates the associated index and erase
        /// values. Assumes that dest points to a position in data[] that is
        /// currently unused. Assumes that from points to a position in data[]
        /// that is in use.
        ///
        /// \param from Pointer to element to move construct from
        /// \param dest Pointer to desired move-construction point
        ///
        void move_construct_element(const pointer from, const pointer dest) {
            //TODO: Implement
        }

        /// Move assigns an element within the container from it's current
        /// position to dest, updates its index, and updates the erase value.
        /// Assumes that dest points to a position in data[] that is currently
        /// being used by an element.
        void move_assign_element(pointer from, pointer dest) {
            *dest = std::move(*from);
            metadata_of(dest).anchor = metadata_of(from).anchor;
            release_anchor(&metadata_of(dest));
        }

        /// Swaps the position of two elements along with their associated
        /// erase and index values.
        ///
        void swap_elements(pointer a, pointer b) noexcept {
            std::swap(metadata_of(a).anchor_index, metadata_of(b).anchor_index);
            std::swap(*a, *b);
        }

        //=================================================
        // Allocation helper methods
        //=================================================

        [[nodiscard]]
        Allocation allocate(const size_type n) {
            Allocation ret{};
            auto md_allocator = md_allocator_type{allocator};

            try {
                ret.elements = allocator_traits::allocate(allocator, n);
                ret.metadata = md_allocator_traits::allocate(md_allocator, n);
                ret.capacity = n;
            } catch (...) {
                allocator_traits::deallocate(allocator, ret.elements, n);
                md_allocator_traits::deallocate(md_allocator, ret.metadata, n);
                ret = {};
                throw;
            }

            return ret;
        }

        /// \param n     Number of elements to allocate memory for
        /// \param alloc Allocator
        /// \return      Allocation object for new object
        [[nodiscard]]
        Allocation allocate(const size_type n, const Allocation& hint) {
            Allocation ret{};
            auto md_allocator = md_allocator_type{allocator};

            try {
                ret.elements = allocator_traits::allocate(allocator, n, hint.elements);
                ret.metadata = md_allocator_traits::allocate(md_allocator, n, hint.metadata);
                ret.capacity = n;

            } catch (...) {
                allocator_traits::deallocate(allocator, ret.elements, n);
                md_allocator_traits::deallocate(md_allocator, ret.metadata, n);
                ret = {};

                throw;
            }

            return ret;
        }

        void deallocate(Allocation& a) {
            auto md_allocator = md_allocator_type{allocator};

            allocator_traits::deallocate(allocator, a.elements, a.capacity);
            md_allocator_traits::deallocate(md_allocator, a.metadata, a.capacity);

            allocation = {};
        }

    };

    template<class T, class A>
    class Slot_map<T, A>::Allocation {
    public:

        //=============================================
        // Instance variables
        //=============================================

        md_pointer metadata = nullptr;
        pointer elements = nullptr;

        size_type capacity = 0;

        //=============================================
        // -ctors
        //=============================================

        Allocation() = default;

        Allocation(const Allocation&) = delete;

        Allocation(Allocation&& alloc) noexcept :
            metadata(std::move(alloc.metadata)),
            elements(std::move(alloc.elements)),
            capacity(std::move(alloc.capacity)) {

            alloc = {};
        }

        ~Allocation() = default;

        //=============================================
        // Assignment operators
        //=============================================

        Allocation& operator=(const Allocation&) = delete;

        Allocation& operator=(Allocation&& alloc) noexcept {
            metadata = std::move(alloc.metadata);
            elements = std::move(alloc.elements);

            capacity = std::move(alloc.capacity);

            alloc.metadata = nullptr;
            alloc.elements = nullptr;
            capacity = 0;

            return *this;
        }

    };

    template<class T, class A>
    class Slot_map<T, A>::Metadata {
    public:

        //=============================================
        // -ctors
        //=============================================

        Metadata() = default;

        Metadata(const size_type anchor):
            anchor(anchor) {}

        Metadata(const size_type i, const size_type anchor):
            anchor_index(i),
            anchor(anchor) {}

        Metadata(const Metadata&) = default;
        Metadata(Metadata&&) = default;

        ~Metadata() = default;

        //=============================================
        // Assignment operators
        //=============================================

        Metadata& operator=(const Metadata&) = default;
        Metadata& operator=(Metadata&&) = default;

        //=============================================
        // Instance members
        //=============================================

        size_type anchor_index{};
        aul::Versioned_type<size_type, size_type> anchor{0, 0};

    };

}

#endif

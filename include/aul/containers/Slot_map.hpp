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
#include <stdexcept>

namespace aul {

    template<class T, class A>
    class Slot_map;


    ///
    /// A class representing a key used by an aul::Slot_map container.
    ///
    /// \tparam T An integral type
    template<class T>
    struct Slot_map_key {
        static_assert(std::numeric_limits<T>::is_integer);
        static_assert(!std::numeric_limits<T>::is_signed);

        template<class U, class A>
        friend class Slot_map;

        //=================================================
        // -ctors
        //=================================================

        Slot_map_key(const T index, const T version):
            index(index),
            version(version) {}

        Slot_map_key() = default;
        Slot_map_key(const Slot_map_key&) = default;
        Slot_map_key(Slot_map_key&&) noexcept = default;
        ~Slot_map_key() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Slot_map_key& operator=(const Slot_map_key&) = default;
        Slot_map_key& operator=(Slot_map_key&&) noexcept = default;

        //=================================================
        // Comparison operators
        //=================================================

        bool operator==(const Slot_map_key& key) const {
            return (this->index == key.index) && (this->version == key.version);
        }

        bool operator!=(const Slot_map_key& key) const {
            return (this->index != key.index) && (this->version == key.version);
        }

        bool operator<(const Slot_map_key& key) const {
            return (this->index < key.index) || (this->version < key.version);
        }

        bool operator>(const Slot_map_key& key) const {
            return (this->index > key.index) || (this->version > key.version);
        }

        bool operator<=(const Slot_map_key& key) const {
            return (this->index <= key.index) || (this->version <= key.version);
        }

        bool operator>=(const Slot_map_key& key) const {
            return (this->index >= key.index) || (this->version >= key.version);
        }

        //=================================================
        // Instance members
        //=================================================

        T index = std::numeric_limits<T>::max();
        T version = std::numeric_limits<T>::max();

    };





    /// Slot_map
    ///
    /// An associative container offering constant time look-up, insertion, and
    /// deletion. 
    ///
    /// Much like a vector, elements are stored in a contiguous array and
    /// iterating through the container is therefore cache-friendly.
    ///
    /// Additionally, each element is mapped to a unique key object from its
    /// construction to its destruction. This key is returned by the emplace()
    /// method and can be retrieved from an iterator via the get_key() method.
    /// A key is invalidated once the element that it was mapped to no longer
    /// exists within the container. The validity of a key can be checked via
    /// the contains() method.
    ///
    /// Algorithms such as std::sort and std::reverse may be applied to the
    /// contents of this container however all keys are liable to lose their
    /// associations. Keys will still map to valid elements but the resulting
    /// mappings are not predictable.
    ///
    /// A default-constructed value of key_type is very unlikely to map to
    /// any object at any time and thus can effectively be used as a null key.
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

        //=================================================
        // Type aliases
        //=================================================

    public:

        using allocator_type = A;

        using size_type = typename std::allocator_traits<A>::size_type;
        using difference_type = typename std::allocator_traits<A>::difference_type;

        using pointer = typename std::allocator_traits<allocator_type>::pointer;
        using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;

        using value_type = T;
        using key_type = Slot_map_key<size_type>;

        using reference = T&;
        using const_reference = const T&;

        using iterator = Random_access_iterator<pointer>;
        using const_iterator = Random_access_iterator<const_pointer>;

    private:

        using allocator_traits = std::allocator_traits<allocator_type>;

        using md_allocator_type = typename std::allocator_traits<A>::template rebind_alloc<Metadata>;
        using md_allocator_traits = std::allocator_traits<md_allocator_type>;
        using md_pointer = typename md_allocator_traits::pointer;

        //=================================================
        // -ctors
        //=================================================

    public:

        ///
        /// Default constructor
        ///
        Slot_map() noexcept(noexcept(allocator_type{})) = default;

        ///
        /// \param alloc Allocator to copy-construct internal allocators from
        ///
        explicit Slot_map(const allocator_type& alloc):
            allocator(alloc) {}

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
        Slot_map(Slot_map&& right, allocator_type alloc):
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
        Slot_map(const Slot_map& src):
            allocator(allocator_traits::select_on_container_copy_construction(src.allocator)),
            allocation(allocate(src.allocation.capacity)),
            elem_count(src.elem_count),
            free_anchor(allocation.metadata + (src.free_anchor - src.allocation.metadata)) {

            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy constructable.");
            //TODO: Provide strong-exception guarantee

            auto md_allocator = md_allocator_type{allocator};

            aul::uninitialized_copy(src.allocation.elements, src.allocation.elements + src.elem_count, allocation.elements, allocator);
            aul::uninitialized_copy(src.allocation.metadata, src.allocation.metadata + src.elem_count, allocation.metadata, md_allocator);
        }

        ///
        /// \param src Source object
        /// \param alloc Source for copy-construction of internal allocator
        Slot_map(const Slot_map& src, const allocator_type& alloc):
            allocator(alloc),
            allocation(allocate(src.allocation.capacity)),
            elem_count(src.elem_count),
            free_anchor(allocation.metadata + (src.free_anchor - src.allocation.metadata)) {

            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy constructable.");
            //TODO: Provide strong exception guarantee

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
        /// Destructs current contents. Reduces capacity to 0. All keys are
        /// invalidated. Keys issues after a call to clear may be equal to
        /// previously used keys. It is recommended to discard all previous
        /// keys immediately before making a call to this function.
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
        void swap(Slot_map& src) noexcept (aul::is_noexcept_swappable_v<A>) {
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
        friend void swap(Slot_map& l, Slot_map& r) noexcept (aul::is_noexcept_swappable_v<A>) {
            l.swap(r);
        }

        //=================================================
        // Assignment operators
        //=================================================

        ///
        /// \param  src
        /// \return Current object
        Slot_map& operator=(const Slot_map& src) {
            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy-constructible.");

            if (this == &src) {
                return *this;
            }
            //TODO: Provide strong-exception guarantee

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
        Slot_map& operator=(Slot_map&& src) noexcept(aul::is_noexcept_movable_v<A>) {
            if (this == &src) {
                return *this;
            }

            //TODO: Provide strong exception guarantee when not noexcept
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

        //=================================================
        // Element access operators/methods
        //=================================================

        ///
        /// Undefined behavior if key is not valid
        ///
        /// \param x Key mapped to desired element
        /// \return  Reference to element mapped to key x
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
        [[nodiscard]]
        const T& operator[](const key_type key) const {
            size_type index = allocation.metadata[key.index].anchor.data();
            return allocation.elements[index];
        }

        /// \param x Key mapped to desired element
        /// \return  Reference to element mapped to key x
        ///
        [[nodiscard]]
        T& at(const key_type k) {
            if (!contains(k)) {
                throw std::runtime_error("aul::Slot_map::operator[] called with invalid key");
            }

            return operator[](k);
        }

        /// \param x Key mapped to desired element
        /// \return  Reference to element mapped to key x
        ///
        [[nodiscard]]
        const T& at(const key_type k) const {
            if (!contains(k)) {
                throw std::runtime_error("aul::Slot_map::operator[] called with invalid key");
            }

            return operator[](k);
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
        template<class... Args>
        key_type emplace(Args&& ... args) {
            //TODO: Provide strong exception guarantee
            if (size() > max_size() - 1) {
                throw std::length_error("aul::Slot_map grew beyond max size");
            }

            if (size() + 1 <= capacity()) {
                construct_element(allocation.elements + size(), std::forward<Args>(args)...);
            } else {
                //Make new allocation
                Allocation new_allocation = allocate(grow_size(size() + 1));

                try {
                    allocator_traits::construct(allocator, new_allocation.elements + size(), std::forward<Args>(args)...);
                } catch (...) {
                    deallocate(new_allocation);

                    throw;
                }

                aul::uninitialized_move(allocation.elements, allocation.elements + size(), new_allocation.elements, allocator);

                //Temporary allocator for indices
                auto md_allocator = md_allocator_type{allocator};

                free_anchor = extend_metadata(allocation, free_anchor, new_allocation, 1);

                deallocate(allocation);
                allocation = std::move(new_allocation);
            }
            md_pointer md = metadata_of(allocation.elements + size());
            auto md_index = static_cast<size_type>(md - allocation.metadata);

            ++elem_count;

            return key_type{md_index, md->anchor.version()};
        }

        iterator insert(const T& v) {
            emplace(v);
            return iterator{allocation.elements + size() - 1};
        }

        iterator insert(T&& v) {
            emplace(std::forward<T&&>(v));
            return iterator{ allocation.elements + size() - 1 };
        }

        //=================================================
        // Element removal
        //=================================================

        ///
        /// \param key Key mapping to element if
        /// \ret True if an element was removed
        bool erase(const key_type key) noexcept {
            md_pointer md = allocation.metadata + key.index;
            if (md->anchor.version() != key.version) {
                return false;
            }

            auto ptr = allocation.elements + key.index;

            pointer last_ptr = allocation.elements + size() - 1;
            md_pointer last_md = metadata_of(last_ptr);

            *ptr = std::move(*last_ptr);
            last_md->anchor.data() = (ptr - allocation.elements);
            allocation.metadata[ptr - allocation.elements].anchor_index = (last_md - allocation.metadata);

            allocator_traits::destroy(allocator, last_ptr);

            if (free_anchor) {
                md->anchor = free_anchor - allocation.metadata;
            }
            else {
                md->anchor = (md - allocation.metadata);
            }
            free_anchor = md;
            --elem_count;
            return true;
        }

        ///
        /// \param it Valid iterator to element to erase
        ///
        void erase(const_iterator it) noexcept {
            auto ptr = const_cast<pointer>(it.operator->());
            md_pointer md = metadata_of(ptr);

            pointer last_ptr = allocation.elements + size() - 1;
            md_pointer last_md = metadata_of(last_ptr);

            *ptr = std::move(*last_ptr);
            last_md->anchor.data() = (ptr - allocation.elements);
            allocation.metadata[ptr - allocation.elements].anchor_index = (last_md - allocation.metadata);

            allocator_traits::destroy(allocator, last_ptr);

            if (free_anchor) {
                md->anchor = free_anchor - allocation.metadata;
            }
            else {
                md->anchor = (md - allocation.metadata);
            }
            free_anchor = md;
            --elem_count;
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
        size_type max_size() const noexcept {
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
            //TODO: Provide strong exception guarantee

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
            if (new_allocation.elements != allocation.elements) {
                aul::uninitialized_move(allocation.elements, allocation.elements + elem_count, new_allocation.elements, allocator);
            }

            free_anchor = extend_metadata(allocation, free_anchor, new_allocation, 0);

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
        void resize(const size_type n, const T& val); //TODO: Implement

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

        //=================================================
        // Misc. methods
        //=================================================

        /// \param it Iterator to element
        /// \return   key corresponding to element pointed to be it
        ///
        [[nodiscard]]
        key_type get_key(const_iterator it) noexcept {
            const_pointer p = it.operator->();

            const size_type x = allocation.metadata[p - allocation.elements].anchor_index;
            const size_type y = allocation.metadata[p - allocation.elements].anchor.version();

            return key_type{x, y};
        }

        /// \param x Key to be checked
        /// \return  Returns true if the key maps to a valid element
        ///
        [[nodiscard]]
        bool contains(const key_type key) const noexcept {
            return (key.index < allocation.capacity) && (key.version == allocation.metadata[key.index].anchor.version());
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
        pointer data() noexcept {
            return allocation.elements;
        }

        ///
        /// \return Pointer to array containing elements
        ///
        [[nodiscard]]
        const_pointer data() const noexcept {
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

        size_type grow_size(const size_type n) noexcept {
            const size_type double_size = (max_size() / 2) < size() ? max_size() : 2 * capacity();
            return std::max(n, double_size);
        }

        /// Returns the index associated with the element pointed to by ptr
        ///
        /// \param Pointer to element in element array
        [[nodiscard]]
        md_pointer metadata_of(const_pointer ptr) const noexcept {
            return allocation.metadata + allocation.metadata[ptr - allocation.elements].anchor;
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
        void consume_anchor(const size_type pos) noexcept {
            const md_pointer free_ptr = free_anchor;

            //If free list terminates, assign nullptr to free_index, otherwise
            //assign the next node.
            auto free_achor_index = (free_ptr - allocation.metadata);
            if (free_ptr->anchor == free_achor_index) {
                free_anchor = nullptr;
            } else {
                free_anchor = allocation.metadata + (free_ptr->anchor);
            }

            free_ptr->anchor.data() = pos;
            allocation.metadata[pos].anchor_index = (free_ptr - allocation.metadata);
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

        ///
        /// Construct n metadata objects for n elements.
        ///
        /// \param n Number of new metadata objects to construct. Must be at least 1
        void generate_metadata(const size_type n) noexcept {
            auto md_allocator = md_allocator_type{allocator};

            for (size_type i = 0; i != n; ++i) {
                md_allocator_traits::construct(md_allocator, allocation.metadata + i, i, i);
            }

            free_anchor = nullptr;
        }

        ///
        /// \param a Allocation to move metadata into and then extend. Capacity
        /// must be greater than that of current allocation
        /// \param n Number of elements to create metadata for.
        /// \return Pointer to free anchor point
        md_pointer extend_metadata(Allocation& from, md_pointer old_free, Allocation& to, const size_type n = 0) noexcept {
            auto md_allocator = md_allocator_type{allocator};

            //Move originals
            for (size_type i = 0; i < from.capacity; ++i) {
                md_allocator_traits::construct(md_allocator, to.metadata + i, std::move(from.metadata[i]));
                md_allocator_traits::destroy(md_allocator, from.metadata + i);
            }

            //Construct new anchors for n elements
            for (size_type i = from.capacity; i != (from.capacity + n); ++i) {
                md_allocator_traits::construct(md_allocator, to.metadata + i, i, i);
            }

            //Construct unused anchors
            for (size_type i = from.capacity + n; i < (to.capacity - 1); ++i) {
                md_allocator_traits::construct(md_allocator, to.metadata + i, i + 1);
            }

            md_pointer new_free_anchor;
            if (n == (to.capacity - from.capacity)) {
                new_free_anchor = old_free;
            } else {
                size_type old_free_index = (old_free) ? (old_free - from.metadata) : (to.capacity - 1);
                md_allocator_traits::construct(md_allocator, to.metadata + to.capacity - 1, old_free_index);
                new_free_anchor = to.metadata + from.capacity + n;
            }

            return new_free_anchor;
        }

        /// Destroys the element pointed to by p through the allocator and
        /// clears the associated index value.
        /// \param p Pointer to element to be destroyed.
        ///
        void destroy_element(pointer p) noexcept {
            md_pointer md = metadata_of(p);
            release_anchor(md);
            allocator_traits::destroy(allocator, p);
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
            consume_anchor(pos - allocation.elements);
            allocator_traits::construct(allocator, pos, std::forward<Args>(args)...);
        }

        /// Move assigns an element within the container from it's current
        /// position to dest, updates its index, and updates the erase value.
        /// Assumes that dest points to a position in data[] that is currently
        /// being used by an element.
        void move_assign_element(pointer from, pointer dest) {
            *dest = std::move(*from);
            metadata_of(dest)->anchor = metadata_of(from)->anchor;
            release_anchor(metadata_of(dest));
        }

        /// Swaps the position of two elements along with their associated
        /// erase and index values.
        ///
        void swap_elements(pointer a, pointer b) noexcept {
            std::swap(metadata_of(a)->anchor_index, metadata_of(b)->anchor_index);
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

            a = {};
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
            alloc.capacity = 0;

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
            anchor(anchor, 1) {}

        Metadata(const size_type anchor, const size_type i):
            anchor_index(i),
            anchor(anchor, 1) {}

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

        size_type anchor_index = 0;
        aul::Versioned_type<size_type, size_type> anchor{0, 1};

    };

}

#endif

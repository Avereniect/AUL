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
#include <limits.h>
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
    /// id_of() method. A key becomes invalid once the element that it was
    /// mapped to no longer exists within the container. The validity of a key
    /// can be checked via the validate_key() method.
    ///
    /// Operations such as insert(), and erase() operate in linear time and
    /// maintain element stability in accordance with vector's API however
    /// constant-time, unstable alternatives, quick_insert(), quick_erase() are
    /// provided.
    ///
    /// Algorithms such as std::sort and std::reverse may be applied to the
    /// contents of this container however all keys are liable to lose their
    /// associations. Keys will still map to valid elements however, the 
    /// resulting mappings are not predictable.
    ///
    /// Unlike std::vector, Slot_map does not have a shrink_to_fit method as
    /// this could potentially invalidate key associations.
    ///
    /// \tparam T     Element type
    /// \tparam Alloc Allocator type
    template<typename T, class Alloc = std::allocator<T>>
    class Slot_map {
    private:

        struct Allocation;
        struct Allocators;

        // TODO: Implement C++ 20 Ranges

        //=====================================================================
        // Type aliases
        //=====================================================================

    public:
        using value_type = T;

        using size_type       = typename std::allocator_traits<Alloc>::size_type;
        using difference_type = typename std::allocator_traits<Alloc>::difference_type;

        using pointer       = typename std::allocator_traits<Alloc>::pointer;
        using const_pointer = typename std::allocator_traits<Alloc>::const_pointer;

    private:

        using index_type = aul::Versioned_type<size_type, size_type>;
        using erase_type = size_type;
        
        using elems_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
        using index_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<index_type>;
        using erase_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<erase_type>;

        using elems_allocator_traits = std::allocator_traits<elems_allocator_type>;
        using index_allocator_traits = std::allocator_traits<index_allocator_type>;
        using erase_allocator_traits = std::allocator_traits<erase_allocator_type>;

        using elems_pointer = pointer;
        using index_pointer = typename index_allocator_traits::pointer;
        using erase_pointer = typename erase_allocator_traits::pointer;

        static_assert(std::is_same<
            typename elems_allocator_traits::size_type,
            typename index_allocator_traits::size_type
        >::value);

        static_assert(std::is_same<
            typename elems_allocator_traits::size_type,
            typename erase_allocator_traits::size_type
        >::value);

    public:

        using key_type = aul::Versioned_type<size_type, size_type>;

        using reference       = T&;
        using const_reference = const T&;

        using allocator_type = Alloc;

        using iterator       = Random_access_iterator<typename aul::Allocator_types<Alloc>, false>;
        using const_iterator = Random_access_iterator<typename aul::Allocator_types<Alloc>, false>;

        using reverse_iterator = typename std::reverse_iterator<iterator>;
        using const_reverse_iterator = typename std::reverse_iterator<const_iterator>;

        //=====================================================================
        // -ctors
        //=====================================================================
        
    public:

        ///
        /// Default constructor
        ///
        Slot_map() noexcept(noexcept(Alloc())) :
            allocators(),
            allocation(),
            elem_count(0),
            free_index(nullptr) {
        }

        /// Allocator extended constructor
        ///
        /// \param alloc Allocator to copy-construct internal allocators from
        ///
        explicit Slot_map(const allocator_type& alloc) noexcept :
            allocators(alloc),
            allocation(),
            elem_count(0),
            free_index(nullptr) {
        }

        ///
        /// \param n     Number of copies made of val
        /// \param val   Source for copy-construction of elements
        /// \param alloc Source for copy-construction of internal allocator
        ///
        Slot_map(const size_type n, const T& val, const Alloc& alloc = {}) :
            allocators(alloc),
            allocation(allocators.allocate(n)),
            elem_count(n),
            free_index(nullptr) {

            aul::uninitialized_fill(allocation.elems_array, allocation.elems_array + n, val, allocators.elems_allocator);
            aul::uninitialized_iota(allocation.index_array, allocation.index_array + n,   0, allocators.index_allocator);
            aul::uninitialized_iota(allocation.erase_array, allocation.erase_array + n,   0, allocators.erase_allocator);
        }

        ///
        /// \param n     Number of elements to default construct
        /// \param alloc Source for copy-construction of internal allocator
        ///
        Slot_map(const size_type n, const Alloc& alloc = {}) :
            allocators(alloc),
            allocation(allocators.allocate(n)),
            elem_count(n),
            free_index(nullptr) {

            aul::default_construct(allocation.elems_array, allocation.elems_array + n, allocators.elems_allocator);
            aul::uninitialized_fill(allocation.index_array, allocation.index_array + n, 0, allocators.index_allocator);
            aul::uninitialized_iota(allocation.erase_array, allocation.erase_array + n, 0, allocators.erase_allocator);
        }

        ///
        /// \param begin Iterator to first element in source range
        /// \param end   Iterator to one past the last element in source range
        /// \param alloc Source for c
        ///
        template<class InputIter>
        Slot_map(InputIter begin, InputIter end, const Alloc& alloc = {}) :
            allocators(alloc),
            allocation(allocators.allocate(end - begin)),
            elem_count(allocation.capacity),
            free_index(nullptr) {

            aul::uninitialized_copy(begin, end, allocation.elems_array, allocators.elems_allocator);
            aul::uninitialized_iota(allocation.index_array, allocation.index_array + elem_count, 0, allocators.index_allocator);
            aul::uninitialized_iota(allocation.erase_array, allocation.erase_array + elem_count, 0, allocators.erase_allocator);
        }

        /// 
        /// \param list  Source list for copy construction of elements
        /// \param alloc Source for copy constructing internal allocators
        /// 
        Slot_map(const std::initializer_list<T> list, Alloc allocator = {}) :
            Slot_map(list.begin(), list.end(), allocator)
        {}

        ///
        /// \param right Source object
        ///
        Slot_map(Slot_map&& right) noexcept :
            allocators(std::move(right.allocators)),
            allocation(std::move(right.allocation)),
            elem_count(std::move(right.elem_count)),
            free_index(std::move(right.free_index)) {

            right.elem_count = 0;
            right.free_index = nullptr;
        }

        ///
        /// \param right Source object
        /// \param alloc Source for copy-construction of internal allocator
        ///
        Slot_map(Slot_map&& right, allocator_type alloc) noexcept :
            allocators(alloc),
            allocation( (alloc == right.get_allocator()) ? std::move(right.allocation) :  allocators.allocate(right.capacity())),
            elem_count(right.elem_count),
            free_index(allocation.index_array + (right.free_index - right.allocation.index_array) ) {

            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy constructible.");

            right.elem_count = 0;
            right.free_index = nullptr;
        }

        ///
        /// \param src Source object
        ///
        Slot_map(const Slot_map& src) :
            allocators(src.allocators),
            allocation(allocators.allocate(src.allocation.capacity)),
            elem_count(src.elem_count),
            free_index(allocation.index_array + (src.free_index - src.allocation.index_array)) {

            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy constructible.");

            aul::uninitialized_copy(src.allocation.elems_array, src.allocation.elems_array + src.elem_count, allocation.elems_array, allocators.elems_allocator);
            aul::uninitialized_copy(src.allocation.index_array, src.allocation.index_array + src.elem_count, allocation.index_array, allocators.index_allocator);
            aul::uninitialized_copy(src.allocation.erase_array, src.allocation.erase_array + src.elem_count, allocation.erase_array, allocators.erase_allocator);
        }

        ///
        /// \param src Source object
        /// \param alloc Source for copy-construction of internal allocator
        ///
        Slot_map(const Slot_map& src, allocator_type alloc) :
            allocators(alloc),
            allocation(allocators.allocate(src.allocation.capacity)),
            elem_count(src.elem_count),
            free_index(allocation.index_array + (src.free_index - src.allocation.index_array)) {

            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy constructible.");

            aul::uninitialized_copy(src.allocation.elems_array, src.allocation.elems_array + elem_count, allocation.elems_array, allocators.elems_allocator);
            aul::uninitialized_copy(src.allocation.index_array, src.allocation.index_array + elem_count, allocation.index_array, allocators.index_allocator);
            aul::uninitialized_copy(src.allocation.erase_array, src.allocation.erase_array + elem_count, allocation.erase_array, allocators.erase_allocator);
        }

        ///
        /// Destructor
        ///
        ~Slot_map() {
            clear();
        }

        //=====================================================================
        // Modifier methods
        //=====================================================================

        ///
        /// Destructs current contents. Reduces capacity to 0.
        ///
        void clear() noexcept {
            if (allocation.capacity) {
                aul::destroy(allocation.elems_array, allocation.elems_array + elem_count, allocators.elems_allocator);
                aul::destroy(allocation.index_array, allocation.index_array + allocation.capacity, allocators.index_allocator);
                aul::destroy(allocation.erase_array, allocation.erase_array + elem_count, allocators.erase_allocator);
            }

            allocators.deallocate(allocation);

            allocation.clear();
            elem_count = 0;
            free_index = nullptr;
        }

        /// Replaces the contents of the current object those of src. Also swaps
        /// allocators if necessary.
        ///
        /// \param src Target object to swap with
        ///
        void swap(Slot_map& src) noexcept (
            std::allocator_traits<Alloc>::propagate_on_container_swap::value ||
            std::allocator_traits<Alloc>::is_always_equal::value) {

            std::swap(allocators, src.allocators);
            std::swap(allocation, src.allocation);
            std::swap(elem_count, src.elem_count);
            std::swap(free_index, src.free_index);
        }

        //=====================================================================
        // Assignment methods & operators
        //=====================================================================

        /// Replaces current content with those in list. New contents are copy-
        /// constructed from originals.
        ///
        /// \param list Source list for elements
        ///
        void assign(const std::initializer_list<T> list) {
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
            clear();
            reserve(end - begin);

            free_index = nullptr;
            elem_count = allocation.capacity;

            aul::uninitialized_copy(begin, end, allocation.elems_array, allocators.elems_allocator);
            aul::uninitialized_iota(allocation.index_array, allocation.index_array + elem_count, 0, allocators.index_allocator);
            aul::uninitialized_iota(allocation.erase_array, allocation.erase_array + elem_count, 0, allocators.erase_allocator);
        }

        /// Replaces current contents with n copies of val.
        ///
        /// \param n   Number of copies made of val
        /// \param val Source for copy-construction of new elements
        ///
        void assign(const size_type n, const T& val) {
            clear();
            reserve(n);

            free_index = nullptr;
            elem_count = n;

            aul::uninitialized_fill(allocation.elems_array, allocation.elems_array + n, val, allocators.elems_allocator);
            aul::uninitialized_iota(allocation.index_array, allocation.index_array + n,   0, allocators.index_allocator);
            aul::uninitialized_iota(allocation.erase_array, allocation.erase_array + n,   0, allocators.erase_allocator);
        }

        /// Copy assignment operator
        ///
        /// \param  src 
        /// \return Current object
        ///
        Slot_map& operator=(Slot_map& src) {
            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy-constructible.");

            if (this == &src) {
                return *this;
            }

            clear();

            allocators = src.allocators;
            allocation = allocators.allocate(src.allocation.capacity);
            elem_count = src.elem_count;
            free_index = allocation.index_array + (src.free_index - src.allocation.index_array);

            aul::uninitialized_copy(src.allocation.elems_array, src.allocation.elems_array + elem_count, allocation.elems_array, allocators.elems_allocator);
            aul::uninitialized_copy(src.allocation.index_array, src.allocation.index_array + elem_count, allocation.index_array, allocators.index_allocator);
            aul::uninitialized_copy(src.allocation.erase_array, src.allocation.erase_array + elem_count, allocation.erase_array, allocators.erase_allocator);

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

            allocators = std::move(src.allocators);
            allocation = std::move(allocation);
            elem_count = std::move(elem_count);
            free_index = std::move(free_index);
            
            src.elem_count = 0;
            src.free_index = nullptr;

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

        //=====================================================================
        // Access methods
        //=====================================================================

        ///
        /// \return Reference to first element in slot map. Undefined if empty
        ///
        [[nodiscard]]
        reference front() {
            return allocation.elems_array[0];
        }

        ///
        /// \return Reference to first element in slot map. Undefined if empty
        ///
        [[nodiscard]]
        const_reference front() const {
            return allocation.elems_array[0];
        }

        ///
        /// \return Reference to last element in slot map. Undefined if empty
        ///
        [[nodiscard]]
        reference back() {
            return allocation.elems_array[elem_count - 1];
        }

        ///
        /// \return Reference to last element in slot map. Undefined if empty
        ///
        [[nodiscard]]
        const_reference back() const {
            return allocation.elems_array[elem_count - 1];
        }

        ///
        /// \param x Index of desired element
        /// \return  Reference to element at index x
        ///
        [[nodiscard]]
        reference at(const size_type x) {
            if (size() <= x) {
                throw std::out_of_range("Index out of bounds.");
            }

            return allocation.elems_array[x];
        }

        ///
        /// \param x Index of desired element
        /// \return  Reference to element at index x
        ///
        [[nodiscard]]
        const_reference at(const size_type x) const {
            if (size() <= x) {
                throw std::out_of_range("Index out of bounds.");
            }

            return allocation.elems_array[x];
        }

        ///
        /// \param x Key mapped to desired element
        /// \return  Reference to element mapped to key x
        ///
        [[nodiscard]]
        reference at(const key_type k) {
            if (size() <= k.data()) {
                throw std::out_of_range("Index out of bounds.");
            }

            return operator[](k);
        }

        ///
        /// \param x Key mapped to desired element
        /// \return  Reference to element mapped to key x
        ///
        [[nodiscard]]
        const_reference at(const key_type k) const {
            if (size() <= k.data()) {
                throw std::out_of_range("Index out of bounds.");
            }

            return operator[](k);
        }

        //=====================================================================
        // Access operators
        //=====================================================================

        ///
        /// \param x Index of desired element
        /// \return  Reference to element at index x
        ///
        [[nodiscard]]
        reference operator[](const size_type x) {
            return allocation.elems_array[x];
        }

        ///
        /// \param x Index of desired element
        /// \return  Reference to element at index x
        ///
        [[nodiscard]]
        const_reference operator[](const size_type x) const {
            return allocation.elems_array[x];
        }

        ///
        /// \param x Key mapped to desired element
        /// \return  Reference to element mapped to key x
        ///
        [[nodiscard]]
        reference operator[](const key_type x) {
            index_type index = allocation.index_array[x.data()];
            return allocation.elems_array[index.data()];
        }

        ///
        /// \param x Key mapped to desired element
        /// \return  Reference to element mapped to key x
        ///
        [[nodiscard]]
        const_reference operator[](const key_type x) const {
            index_type index = allocation.index_array[x.data()];
            return allocation.elems_array[index.data()];
        }

        //=====================================================================
        // Element mutators
        //=====================================================================

        iterator insert(const_iterator pos, const T& value);

        iterator insert(const_iterator pos, T&& value);

        iterator insert(const_iterator pos, const size_type count, const T& value);

        template<class InputIter>
        iterator insert(iterator pos, InputIter begin, InputIter end);

        iterator insert(const_iterator pos, std::initializer_list<T> list);



        iterator quick_insert(const_iterator pos, const T& value) {
            reserve(size() + 1);
            
            pointer pos_ptr = std::addressof(*pos);

            move_construct_element(pos_ptr, allocation.elems_array + elem_count);
            construct_element(pos_ptr, value);

            ++elem_count;
        }

        ///
        /// \param pos   Iterator to position to insert at
        /// \param value Value to move construct from
        ///
        iterator quick_insert(const_iterator pos, T&& value) {
            reserve(size() + 1);

            pointer pos_ptr = std::addressof(*pos);

            move_construct_element(pos_ptr, allocation.elems_array + elem_count);
            construct_element(pos_ptr, std::forward<T&&>(value));

            ++elem_count;
        }

        ///
        /// \param pos   Iterator to position to begin inserting at
        /// \param count Number of elements to be inserted
        /// \param value Object from which elements are inserted
        ///
        iterator quick_insert(const_iterator pos, const size_type count, const T& value);

        ///
        /// \tparam Input_iter Type of iterator specifying source range
        /// \param pos   Iterator to position to begin inserting at
        /// \param begin Iterator to begining of source range
        /// \param end   Iterator to end of source range
        ///
        template<class Input_iter>
        iterator quick_insert(iterator pos, Input_iter begin, Input_iter end);

        ///
        /// \param pos  Iterator to position to begin inserting at
        /// \param list Initiializer list to copy-construct elements from
        ///
        iterator quick_insert(const_iterator pos, std::initializer_list<T> list) {
            quick_insert(pos, list.begin(), list.end());
        }



        ///
        /// \param key Key mapping to element to erase
        ///
        void erase(const key_type key);

        ///
        /// \param it Iterator to element to erase
        ///
        void erase(const_iterator it);

        ///
        /// \param key Key mapping to element to erase
        ///
        void erase_quick(const key_type key);

        ///
        /// \param it Iterator to element to erase
        ///
        void erase_quick(const_iterator it);

        ///
        /// \param val Object to copy-construct from
        ///
        void push_back(const T& val) {
            emplace_back(val);
        }

        ///
        /// \param val Object to move-construct from
        ///
        void push_back(T&& val) {
            emplace_back(std::forward<T&&>(val));
        }

        ///
        /// Removes the last element in the container. undefined if container
        /// is empty.
        ///
        void pop_back() {
            --elem_count;
            destroy_element(allocation.elems_array + elem_count);
        }

        ///
        /// Constructs an object from a set of paramaters at the last position
        /// in the container.
        ///
        /// \tparam Args Argument types for constructor call
        /// \param args  Constructor arguments for construction of new element
        /// \return      Reference to newly constructed object
        ///
        template<class... Args>
        reference emplace_back(Args&& ... args) {
            //TODO: Conditionally provide the strong-exception guarantee
            //If resize neccessary:
            //Try:
            //Make new allocation
            //Construct new element
            //Move elements to new allocation
            //Release previous allocation
            //Catch:
            //Undo moving of elements
            //Release previous allocation

            reserve(size() + 1);

            construct_element(
                allocation.elems_array + elem_count,
                std::forward<Args>(args)...
            );

            ++elem_count;
            return allocation.elems_array[elem_count - 1];
        }

        ///
        /// Constructs an object from a set of parameters at a specified place
        /// Is stable.
        ///
        /// \tparam Args Argument types for constructor call
        /// \param pos   Iterator to desired emplacement point
        /// \param args  Constructor arguments for construction of new element
        /// \return      Reference to newly constructed object
        ///
        template<class... Args>
        reference emplace(const_iterator pos, Args&& ... args);

        ///
        /// Constructs an object from a set of parameter at a specified place
        /// Is not stable.
        ///
        /// \tparam Args Argument types for constructor call
        /// \param pos   Iterator to desired emplacement point
        /// \param args  Constructor arguments for construction of new element
        /// \return      Reference to newly constructed object
        ///
        template<class... Args>
        reference emplace_quick(const_iterator pos, Args&& ... args);

        //=====================================================================
        // Iterator methods
        //=====================================================================

        iterator begin() noexcept {
            return iterator(allocation.elems_array);
        }

        const_iterator end() noexcept {
            const_iterator it(allocation.elems_array ? allocation.elems_array + (elem_count - 1) : nullptr);
            return it;
        }
        
        const_iterator begin() const noexcept {
            return const_iterator(allocation.elems_array);
        }


        iterator end() const noexcept {
            iterator it(allocation.elems_array ? allocation.elems_array + elem_count : nullptr);
            return it;
        }

        const_iterator cbegin() const noexcept {
            return const_cast<const Slot_map&>(*this).begin();
        }

        const_iterator cend() const noexcept {
            return const_cast<const Slot_map&>(*this).end();
        }


        reverse_iterator rbegin() noexcept {
            return reverse_iterator(end());
        }

        reverse_iterator rend() noexcept {
            return reverse_iterator(begin());
        }

        const_reverse_iterator rbegin() const noexcept {
            return reverse_iterator(end());
        }

        const_reverse_iterator rend() const noexcept {
            return reverse_iterator(begin());
        }

        const_reverse_iterator crbegin() const noexcept {
            return const_cast<const Slot_map&>(*this).rbegin();
        }

        const_reverse_iterator crend() const noexcept {
            return const_cast<const Slot_map&>(*this).rend();
        }

        //=====================================================================
        // Size & capacity methods
        //=====================================================================

        ///
        /// \return size() == 0
        ///
        [[nodiscard]]
        bool empty() const noexcept {
            return size() == 0;
        }

        ///
        /// \return Allocation capacity
        ///
        [[nodiscard]]
        size_type capacity() const noexcept {
            return allocation.capacity;
        }

        ///
        /// \return Element count
        ///
        [[nodiscard]]
        size_type size() const noexcept {
            return elem_count;
        }

        ///
        /// \return Maximum capacity container may reach.
        ///
        [[nodiscard]]
        size_type max_size() const noexcept {
            constexpr size_type size_type_max = std::numeric_limits<difference_type>::max();
            const size_type element_max = sizeof(value_type) * elems_allocator_traits::max_size(allocators.elems_allocator);

            const size_type memory_max = element_max  / (sizeof(value_type) + sizeof(erase_type) + sizeof(index_type));

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

            Allocation new_memory = allocators.allocate(n);

            if (new_memory.index_array != allocation.index_array) {
                aul::uninitialized_move(allocation.index_array, allocation.index_array + elem_count, new_memory.index_array, allocators.index_allocator);
            }

            if (new_memory.elems_array != allocation.elems_array) {
                aul::uninitialized_move(allocation.elems_array, allocation.elems_array + elem_count, new_memory.elems_array, allocators.elems_allocator);
            }
            
            if (new_memory.erase_array != allocation.erase_array) {
                aul::uninitialized_move(allocation.erase_array, allocation.erase_array + elem_count, new_memory.erase_array, allocators.erase_allocator);
            }

            //Create new empty indices
            aul::uninitialized_iota(
                new_memory.index_array + allocation.capacity,
                new_memory.index_array + new_memory.capacity,
                allocation.capacity + 1,
                allocators.index_allocator
            );

            if (free_index) {
                //Append previous list head to end of free list
                new_memory.index_array[new_memory.capacity - 1].data() = free_index - allocation.index_array;
            } else {
                //Terminate free list
                new_memory.index_array[new_memory.capacity - 1].data() = new_memory.capacity - 1;
            }

            free_index = new_memory.index_array + (free_index - allocation.index_array);

            allocators.deallocate(allocation);
            allocation = std::move(new_memory);
        }

        ///
        /// Resizes the container to contain exact n many elements. If n is 
        /// less than the current size, the excess objects are deleted. If n is
        /// greater than the current size default constructed elements are
        /// inserted to the end of the container.
        ///
        void resize(const size_type n) {
            resize(n, value_type{});
        }

        ///
        /// Resizes the container to contain exact n many elements. If n is 
        /// less than the current size, the excess objects are deleted. If n is
        /// greater than the current size copies of val are inserted to the end
        /// of the container.
        ///
        void resize(const size_type n, const_reference val) {
            if (size() < n) {
                for (iterator it = end() - 1; it != begin() + n; --it) {
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

            if (size() == n) {
                return;
            }
        }

        //=====================================================================
        // Comparison operators
        //=====================================================================

        [[nodiscard]]
        friend bool operator==(const Slot_map& lhs, const Slot_map& rhs) {
            return std::equal(lhs.begin(), lhs.begin(), rhs.begin(), rhs.end());
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

        //=====================================================================
        // Misc. methods
        //=====================================================================

        ///
        /// \param it Iterator to element
        /// \return   key corresponding to element indicates by it
        ///
        [[nodiscard]]
        key_type id_of(const_iterator it) {
            const_pointer p = std::addressof(*it);

            return {
                allocation.erase_array[p - allocation.elems_array],
                allocation.index_array[p - allocation.elems_array].version()
            };
        }

        ///
        /// \param x Key to be checked
        /// \return  Returns true if the key maps to a valid element
        ///
        [[nodiscard]]
        bool validate_id(key_type x) noexcept {
            return 
                x.version() == allocation.index_array[x.data()].version() &&
                x.data() <= size();
        }

        ///
        /// \return Copy of internal allocator
        ///
        [[nodiscard]]
        Alloc get_allocator() const noexcept {
            return allocators.elems_allocator;
        }

        ///
        /// \return Pointer to array containing elements
        ///
        [[nodiscard]]
        pointer data() noexcept {
            return allocation.elems_array;
        }

        ///
        /// \return Pointer to array containing elements
        ///
        [[nodiscard]]
        const_pointer data() const noexcept {
            return allocation.elems_array;
        }

    private:

        //=====================================================================
        // Instance members
        //=====================================================================

        Allocators    allocators;

        Allocation    allocation;

        size_type     elem_count;

        index_pointer free_index;

        //=====================================================================
        // Helper functions
        //=====================================================================

        /// Increases the capacity of the 
        ///
        /// \param n Number of elements 
        ///
        void grow(size_type n) {
            constexpr size_type size_type_max = std::numeric_limits<size_type>::max();
            const size_type double_size = (size_type_max / 2) < size() ? size_type_max : 2 * size();

            reserve(std::max(n, double_size));
        }

        /// Take an free index and populates it to point at the position
        /// indicated by pos.
        ///
        /// \pre An index that is free must exist. e.g. free_index != nullptr
        ///
        /// \return Pointer to index that has been consumed.
        ///
        index_pointer consume_index(const size_type pos) noexcept {
            const index_pointer free_ptr = free_index;

            if (free_ptr->data() == (free_ptr - allocation.index_array) ) {
                free_index = nullptr;
            } else {
                free_index = allocation.index_array + (free_ptr->data());
            }

            free_ptr->data() = pos;

            return free_ptr;
        }

        ///
        /// Frees index pointed to by ptr and pushes it onto list of free
        /// indices. Increments index version.
        ///
        void release_index(const index_pointer ptr) noexcept {
            if (free_index) {
                *ptr = free_index - allocation.index_array;
            } else {
                *ptr = ptr - allocation.index_array;
            }
            free_index = ptr;
        }

        ///
        /// Returns the index associated with the element pointed to by ptr
        /// 
        /// \param Pointer to element in element array
        ///
        [[nodiscard]]
        index_type& index_of(const_pointer ptr) const noexcept {
            return allocation.index_array[allocation.erase_array[ptr - allocation.elems_array]];
        }

        /// 
        /// 
        /// \param ptr Pointer to element in elems_array
        ///
        /// \return Reference to the erase value associated with the 
        [[nodiscard]]
        erase_type& erase_of(const_pointer ptr) const noexcept {
            return allocation.erase_array[ptr - allocation.elems_array];
        }

        /// Destroys the element pointed to by p through the allocator and
        /// clears the associated inde value.
        /// \param p Pointer to element to be destroyed.
        ///
        void destroy_element(pointer p) {
            release_index(std::addressof(index_of(p)));
            erase_allocator_traits::destroy(allocators.erase_allocator, std::addressof(erase_of(p)));
            elems_allocator_traits::destroy(allocators.elems_allocator, p);
        }

        /// Constructs an element at the specified position along with the
        /// corresponding erase value and index
        ///
        /// \pre mem.free_head points to an already constructed index object
        ///
        /// \tparam     Args args type
        /// \param p    Pointer to desired construction point
        /// \param args Constructor parameters
        template<class...Args>
        void construct_element(pointer pos, Args&& ... args) {
            index_pointer index = free_index;

            consume_index(pos - allocation.elems_array);

            elems_allocator_traits::construct(
                allocators.elems_allocator,
                pos,
                std::forward<Args>(args)...
            );

            erase_allocator_traits::construct(
                allocators.erase_allocator,
                allocation.erase_array + (pos - allocation.elems_array),
                index - allocation.index_array
            );
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
        void move_construct_element(pointer from, pointer dest) {
            //Can probably be micro-optimized
            index_type& from_index = index_of(from);
            index_type& dest_index = index_of(dest);

            erase_type& from_erase = erase_of(from);
            erase_type& dest_erase = erase_of(dest);

            dest_index.data() = std::move(from_index.data());
            release_index(&from_index);

            erase_allocator_traits::construct(allocators.erase_allocator, &dest_erase, std::move(from_erase));
            erase_allocator_traits::destroy(allocators.erase_allocator, &from_erase);

            elems_allocator_traits::construct(allocators.elems_allocator, dest, std::move(*from));
            elems_allocator_traits::destroy(allocators.elems_allocator, from);
        }

        /// Move assigns an element within the container from it's current
        /// position to dest, updates its index, and updates the erase value.
        /// Assumes that dest points to a position in data[] that is currently
        /// being used by an element. 
        ///
        void move_assign_element(pointer from, pointer dest) {
            *dest = std::move(*from);
            index_of(dest).data() = index_of(from);
            erase_of(dest) = std::move(erase_of(from));
            release_index(&index_of(dest));
        }

        ///
        /// Swaps the position of two elements along with their associated
        /// erase and index values.
        ///
        void swap_elements(pointer a, pointer b) noexcept {
            std::swap(index_of(a).data(), index_of(b).data());
            std::swap(*a, *b);
            std::swap(erase_of(a), erase_of(b));
        }

        //=====================================================================
        // Helper classes
        //=====================================================================

        ///
        /// Wrapper class around all memory allocators needed by Slot_map
        /// class. Abstracts away need for handling copy and move semantics
        /// as well as constructions of allocators individually. 
        ///
        struct Allocators {

            //---------------------------------------------
            // Instance members
            //---------------------------------------------

            elems_allocator_type elems_allocator;
            index_allocator_type index_allocator;
            erase_allocator_type erase_allocator;

            //---------------------------------------------
            // -ctors
            //---------------------------------------------

            Allocators() = default;

            Allocators(allocator_type alloc) :
                elems_allocator(alloc),
                index_allocator(alloc),
                erase_allocator(alloc) {
            }

            Allocators(const Allocators& allocs) :
                elems_allocator(elems_allocator_traits::select_on_container_copy_construction(allocs.elems_allocator)),
                index_allocator(index_allocator_traits::select_on_container_copy_construction(allocs.index_allocator)),
                erase_allocator(erase_allocator_traits::select_on_container_copy_construction(allocs.erase_allocator)){
            }

            Allocators(Allocators&& allocs) = default;

            ~Allocators() = default;

            //---------------------------------------------
            // Assignment operators
            //---------------------------------------------

            Allocators& operator=(const Allocators& allocs) {
                if constexpr (elems_allocator_traits::propagate_on_container_copy_assignment::value) {
                    elems_allocator = allocs.elems_allocator;
                }

                if constexpr (index_allocator_traits::propagate_on_container_copy_assignment::value) {
                    index_allocator = allocs.index_allocator;
                }

                if constexpr (erase_allocator_traits::propagate_on_container_copy_assignment::value) {
                    erase_allocator = allocs.erase_allocator;
                }
            }

            Allocators& operator=(Allocators&& allocs) {
                if constexpr (elems_allocator_traits::propagate_on_container_move_assignment::value) {
                    elems_allocator = std::move(allocs.elems_allocator);
                }

                if constexpr (index_allocator_traits::propagate_on_container_move_assignment::value) {
                    index_allocator = std::move(allocs.index_allocator);
                }

                if constexpr (erase_allocator_traits::propagate_on_container_move_assignment::value) {
                    erase_allocator = std::move(allocs.erase_allocator);
                }
            }

            //---------------------------------------------
            // Swap methods
            //---------------------------------------------

            void swap(Allocators& allocs) {
                if constexpr (elems_allocator_traits::propagate_on_container_swap::value) {
                    std::swap(elems_allocator, allocs.elems_allocator);
                }

                if constexpr (index_allocator_traits::propagate_on_container_swap::value) {
                    std::swap(index_allocator, allocs.index_allocator);
                }

                if constexpr (erase_allocator_traits::propagate_on_container_swap::value) {
                    std::swap(erase_allocator, allocs.erase_allocator);
                }
            }

            //---------------------------------------------
            // Allocation & deallocation methods
            //---------------------------------------------

            Allocation allocate(const size_type count) {
                Allocation mem{};

                try {
                    mem.index_array = index_allocator_traits::allocate(index_allocator, count);
                    mem.elems_array = elems_allocator_traits::allocate(elems_allocator, count);
                    mem.erase_array = erase_allocator_traits::allocate(erase_allocator, count);
                    mem.capacity = count;

                } catch (...) {
                    index_allocator_traits::deallocate(index_allocator, mem.index_array, count);
                    elems_allocator_traits::deallocate(elems_allocator, mem.elems_array, count);
                    erase_allocator_traits::deallocate(erase_allocator, mem.erase_array, count);

                    mem.clear();

                    throw;
                }

                return mem;
            }

            Allocation& allocate(size_type count, Allocation& mem_hint) {
                Allocation mem;
                try {
                    mem.index_array = index_allocator_traits::allocate(index_allocator, count, mem_hint.index_array);
                    mem.elems_array = elems_allocator_traits::allocate(elems_allocator, count, mem_hint.elems_array);
                    mem.erase_array = erase_allocator_traits::allocate(erase_allocator, count, mem_hint.erase_array);
                    mem.capacity = count;

                } catch (...) {
                    index_allocator_traits::deallocate(index_allocator, mem.index_array, count);
                    elems_allocator_traits::deallocate(elems_allocator, mem.elems_array, count);
                    erase_allocator_traits::deallocate(erase_allocator, mem.erase_array, count);

                    mem.clear();

                    throw;
                }

                return mem;
            }

            void deallocate(Allocation& mem) {
                try {
                    index_allocator_traits::deallocate(index_allocator, mem.index_array, mem.capacity);
                    elems_allocator_traits::deallocate(elems_allocator, mem.elems_array, mem.capacity);
                    erase_allocator_traits::deallocate(erase_allocator, mem.erase_array, mem.capacity);
                    mem.clear();

                } catch (...) {
                    throw;
                }
            }

        }; // class aul::Slot_map<T, Alloc>::Allocators

        struct Allocation {

            //---------------------------------------------
            // Instance variables
            //---------------------------------------------

            index_pointer index_array;
            elems_pointer elems_array;
            erase_pointer erase_array;

            size_type capacity;

            //---------------------------------------------
            // -ctors
            //---------------------------------------------

            Allocation() = default;
            Allocation(const Allocation&) = default;
            
            Allocation(Allocation&& alloc) noexcept :
                index_array(std::move(alloc.index_array)),
                elems_array(std::move(alloc.elems_array)),
                erase_array(std::move(alloc.erase_array)),
                capacity(std::move(alloc.capacity)) {

                alloc.clear();
            }

            ~Allocation() = default;

            //---------------------------------------------
            // Assignment operators
            //---------------------------------------------

            Allocation& operator=(const Allocation&) = default;

            Allocation& operator=(Allocation&& alloc) {
                index_array = std::move(alloc.index_array);
                elems_array = std::move(alloc.elems_array);
                erase_array = std::move(alloc.erase_array);

                capacity = std::move(alloc.capacity);

                alloc.clear();

                return *this;
            }

            //---------------------------------------------
            // Misc. methods
            //---------------------------------------------

            ///
            /// Resets all pointers to nullptr and integral values to 0
            ///
            void clear() noexcept {
                index_array = nullptr;
                elems_array = nullptr;
                erase_array = nullptr;

                capacity = 0;
            }

        };// class aul::Slot_map<T, Alloc>::Allocation

    };// class aul::Slot_map<T, Alloc>

}// namespace aul

#endif

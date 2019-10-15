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
    /// contents of this container however keys would likely now reference
    /// different objects lose their former meaning much as indices would
    /// for a std::vector.
    /// 
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
            allocs(Alloc()),
            mem() {

            mem.free_head = nullptr;
            mem.size = 0;
        }

        /// Allocator extended constructor
        ///
        /// \tparam alloc Source allocator
        ///
        explicit Slot_map(const allocator_type& alloc) noexcept :
            allocs(alloc),
            mem() {

            mem.free_head = nullptr;
            mem.size = 0;
        }

        /// Element fill constructor
        ///
        /// \param n     Number of copies made of val
        /// \param val   Source for copy-construction of elements
        /// \param alloc Source for copy-construction of internal allocator
        ///
        Slot_map(const size_type n, const T& val, const Alloc& alloc = {}) :
            allocs(alloc),
            mem(allocs.allocate(n)) {

            mem.free_head = nullptr;
            mem.size = n;

            aul::uninitialized_fill(mem.elems_array, mem.elems_array + mem.capacity, val, allocs.elems_allocator);
            aul::uninitialized_iota(mem.index_array, mem.index_array + mem.capacity,   0, allocs.index_allocator);
            aul::uninitialized_iota(mem.erase_array, mem.erase_array + mem.capacity,   0, allocs.erase_allocator);
        }

        /// Default fill constructor
        ///
        /// \param n     Number of elements to default construct
        /// \param alloc Source for copy-construction of internal allocator
        ///
        Slot_map(const size_type n, const Alloc& alloc = {}) :
            allocs(alloc),
            mem(allocs.allocate(n)) {

            mem.free_head = nullptr;
            mem.size = n;
            aul::default_construct(mem.elems_array, mem.elems_array + n, allocs.elems_allocator);
            aul::uninitialized_fill(mem.index_array, mem.index_array + n, 0, allocs.index_allocator);
            aul::uninitialized_iota(mem.erase_array, mem.erase_array + n, 0, allocs.erase_allocator);
        }

        /// Iterator extended constructor
        ///
        /// \param begin Iterator to first element in source range
        /// \param end   Iterator to one past the last element in source range
        /// \param alloc Source for c
        ///
        template<class InputIter>
        Slot_map(InputIter begin, InputIter end, const Alloc& alloc = {}) :
            allocs(alloc),
            mem(allocs.allocate(end - begin)) {

            mem.free_head = nullptr;
            mem.size = mem.capacity;
            aul::uninitialized_copy(begin, end, mem.elems_array, allocs.elems_allocator);
            aul::uninitialized_iota(mem.index_array, mem.index_array + mem.capacity, 0, allocs.index_allocator);
            aul::uninitialized_iota(mem.erase_array, mem.erase_array + mem.capacity, 0, allocs.erase_allocator);
        }

        /// Initializer list extended constructor
        /// 
        /// \param list  Source list for copy construction of elements
        /// \param alloc Source for copy constructing internal allocators
        /// 
        Slot_map(const std::initializer_list<T> list, Alloc allocator = {}) :
            Slot_map(list.begin(), list.end(), allocator) {}

        /// Move constructor
        ///
        /// \param right Source object
        ///
        Slot_map(Slot_map&& right) noexcept :
            allocs(std::move(right.allocs)),
            mem(std::move(right.mem)) {

            static_assert(std::is_move_constructible<T>::value, "Type T is not move constructible.");
        }

        /// Allocator extended move constructor
        ///
        /// \param right Source object
        /// \param alloc Source for copy-construction of internal allocator
        ///
        Slot_map(Slot_map&& right, Alloc alloc) noexcept :
            allocs(std::move(right.allocs)),
            mem(std::move(right.mem)) {
        }

        /// Copy Constructor
        ///
        /// \param src Source object
        ///
        Slot_map(const Slot_map& src) :
            allocs(src.allocs),
            mem(allocs.allocate(src.mem.size)) {

            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy constructible.");

            mem.free_head = nullptr;
            mem.size = src.size();

            aul::uninitialized_copy(src.mem.elems_array, src.mem.elems_array + mem.size, mem.elems_array, allocs.elems_allocator);
            aul::uninitialized_copy(src.mem.index_array, src.mem.index_array + mem.size, mem.index_array, allocs.index_allocator);
            aul::uninitialized_copy(src.mem.erase_array, src.mem.erase_array + mem.size, mem.erase_array, allocs.erase_allocator);
        }

        /// Allocator extended copy constructor
        ///
        /// \param src Source object
        /// \param alloc Source for copy-construction of internal allocator
        ///
        Slot_map(const Slot_map& src, Alloc alloc) :
            allocs(alloc),
            mem(allocs.allocate(src.mem.size)) {

            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy constructible.");

            mem.free_head = nullptr;
            mem.size = src.mem.size;

            aul::uninitialized_copy(src.mem.elems_array, src.mem.elems_array + mem.size, mem.elems_array, allocs.elems_allocator);
            aul::uninitialized_copy(src.mem.index_array, src.mem.index_array + mem.size, mem.index_array, allocs.index_allocator);
            aul::uninitialized_copy(src.mem.erase_array, src.mem.erase_array + mem.size, mem.erase_array, allocs.erase_allocator);
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
            aul::destroy(mem.elems_array, mem.elems_array + mem.size, allocs.elems_allocator);
            aul::destroy(mem.index_array, mem.index_array + mem.size, allocs.index_allocator);
            aul::destroy(mem.erase_array, mem.erase_array + mem.size, allocs.erase_allocator);

            allocs.deallocate(mem);
            mem.clear();
        }

        /// Replaces the contents of the current object those of src. Also swaps
        /// allocators if necessary.
        ///
        /// \param src Target object to swap with
        ///
        void swap(Slot_map& src) noexcept (
            std::allocator_traits<Alloc>::propagate_on_container_swap::value ||
            std::allocator_traits<Alloc>::is_always_equal::value) {

            std::swap(this->allocs, src.allocs);
            std::swap(this->mem, src.mem);
        }

        //=====================================================================
        // Assignment methods & operators
        //=====================================================================

        /// Replaces current content with those in list. New contents are copy-
        /// constructed from originals.
        ///
        /// \param list Source list for elements
        ///
        inline void assign(const std::initializer_list<T> list) {
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

            mem.free_head = nullptr;
            mem.size = mem.capacity;

            aul::uninitialized_copy(begin, end, mem.elems_array, allocs.elems_allocator);
            aul::uninitialized_iota(mem.index_array, mem.index_array + mem.size, 0, allocs.index_allocator);
            aul::uninitialized_iota(mem.erase_array, mem.erase_array + mem.size, 0, allocs.erase_allocator);
        }

        /// Replaces current contents with n copies of val.
        ///
        /// \param n   Number of copies made of val
        /// \param val Source for copy-construction of new elements
        ///
        void assign(const size_type n, const T& val) {
            clear();
            reserve(n);

            mem.free_head = nullptr;
            mem.size = n;

            aul::uninitialized_fill(mem.elems_array, mem.elems_array + n, val, allocs.elems_allocator);
            aul::uninitialized_iota(mem.index_array, mem.index_array + n,   0, allocs.index_allocator);
            aul::uninitialized_iota(mem.erase_array, mem.erase_array + n,   0, allocs.erase_allocator);
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

            allocs = src.allocs;
            mem = allocs.allocate(src.mem.size);
            mem.size = src.size();
            mem.free_head = mem.index_array + (src.mem.free_head - src.mem.index_array);

            aul::uninitialized_copy(src.mem.elems_array, src.mem.elems_array + mem.size, mem.elems_array, allocs.elems_allocator);
            aul::uninitialized_copy(src.mem.index_array, src.mem.index_array + mem.size, mem.index_array, allocs.index_allocator);
            aul::uninitialized_copy(src.mem.erase_array, src.mem.erase_array + mem.size, mem.erase_array, allocs.erase_allocator);

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

            allocs = std::move(src.allocs);
            mem = std::move(mem);

            return *this;
        }

        /// Replaces current contents with those of list.
        ///
        /// \param list Source for replace
        /// \return  Current object
        ///
        inline Slot_map& operator=(const std::initializer_list<T> list) {
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
        inline reference front() {
            return mem.elems_array[0];
        }

        ///
        /// \return Reference to first element in slot map. Undefined if empty
        ///
        [[nodiscard]]
        inline const_reference front() const {
            return mem.elems_array[0];
        }

        ///
        /// \return Reference to last element in slot map. Undefined if empty
        ///
        [[nodiscard]]
        inline reference back() {
            return mem.elems_array[mem.size - 1];
        }

        ///
        /// \return Reference to last element in slot map. Undefined if empty
        ///
        [[nodiscard]]
        inline const_reference back() const {
            return mem.elems_array[mem.size - 1];
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

            return mem.elems_array[x];
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

            return mem.elems_array[x];
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
            return mem.elems_array[x];
        }

        ///
        /// \param x Index of desired element
        /// \return  Reference to element at index x
        ///
        [[nodiscard]]
        const_reference operator[](const size_type x) const {
            return mem.elems_array[x];
        }

        ///
        /// \param x Key mapped to desired element
        /// \return  Reference to element mapped to key x
        ///
        [[nodiscard]]
        reference operator[](const key_type x) {
            index_type index = mem.index_array[x.data()];
            return mem.elems_array[index.data()];
        }

        ///
        /// \param x Key mapped to desired element
        /// \return  Reference to element mapped to key x
        ///
        [[nodiscard]]
        const_reference operator[](const key_type x) const {
            index_type index = mem.index_array[x.data()];
            return mem.elems_array[index.data()];
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



        iterator quick_insert(const_iterator pos, const T& value);

        iterator quick_insert(const_iterator pos, T&& value);

        iterator quick_insert(const_iterator pos, const size_type count, const T& value);

        template<class InputIter>
        iterator quick_insert(iterator pos, InputIter begin, InputIter end);

        iterator quick_insert(const_iterator pos, std::initializer_list<T> list);



        ///
        /// \param id Key mapping to element to remove
        ///
        void erase(const key_type id);

        ///
        /// \param x Iterator to element to remove
        ///
        void erase(const_iterator x);

        ///
        /// \param val Object to copy-construct from
        ///
        inline void push_back(const T& val) {
            emplace_back(val);
        }

        ///
        /// \param val Object to move-construct from
        ///
        inline void push_back(T&& val) {
            emplace_back(std::move(val));
        }

        ///
        /// Removes the last element in the container. Undefined behavior if
        /// empty.
        ///
        void pop_back(); //TODO: Implement

        ///
        /// \tparam Args Argument types for constructor call
        /// \param args  Constructor arguments for construction of new element
        /// \return      Reference to newly constructed object
        template<class... Args>
        reference emplace_back(Args&& ... args) {
            if (size() + 1 > capacity()) {
                grow(size() + 1);
            }

            emplace_element(
                mem.elems_array + mem.size,
                std::forward<Args>(args)...
            );

            return mem.elems_array[mem.size - 1];
        }

        ///
        /// \tparam Args Argument types for constructor call
        /// \param pos   Iterator to desired emplacement point
        /// \param args  Constructor arguments for construction of new element
        /// \return      Reference to newly constructed object
        template<class... Args>
        reference emplace(const_iterator pos, Args&& ... args);

        // TODO: Implement
        template<class... Args>
        reference emplace_stable(const_iterator pos, Args&& ... args);

        //=====================================================================
        // Iterator methods
        //=====================================================================

        iterator begin() noexcept {
            return iterator(mem.elems_array);
        }
        const_iterator begin() const noexcept {
            return const_iterator(mem.elems_array);
        }

        const_iterator cbegin() const noexcept {
            return const_iterator(mem.elems_array);
        }


        iterator end() noexcept {
            iterator it(mem.elems_array ? mem.elems_array + mem.size : nullptr);
            return it;
        }

        const_iterator end() const noexcept {
            const_iterator it(mem.elems_array ? mem.elems_array + (mem.size - 1) : nullptr);
            return it;
        }

        const_iterator cend() const noexcept {
            const_iterator it(mem.elems_array ? mem.elems_array + (mem.size - 1) : nullptr);
            return it;
        }


        reverse_iterator rbegin() noexcept {
            return reverse_iterator(end());
        }

        const_reverse_iterator rbegin() const noexcept {
            return reverse_iterator(end());
        }

        const_reverse_iterator crbegin() const noexcept {
            return reverse_iterator(end());
        }


        reverse_iterator rend() noexcept {
            return reverse_iterator(begin());
        }

        const_reverse_iterator rend() const noexcept {
            return reverse_iterator(begin());
        }

        const_reverse_iterator crend() const noexcept {
            return reverse_iterator(begin());
        }

        //=====================================================================
        // size & capacity methods
        //=====================================================================

        ///
        /// \return size() == 0
        ///
        [[nodiscard]]
        inline bool empty() const noexcept {
            return mem.size == 0;
        }

        ///
        /// \return Allocation capacity
        ///
        [[nodiscard]]
        size_type capacity() const noexcept {
            return mem.capacity;
        }

        ///
        /// \return Element count
        ///
        [[nodiscard]]
        inline size_type size() const noexcept {
            return mem.size;
        }

        ///
        /// \return Maximum capacity slot map can reach.
        ///
        [[nodiscard]]
        inline size_type max_size() const noexcept {
            constexpr size_type a = std::numeric_limits<difference_type>::max();
            const size_type     b = std::allocator_traits<Alloc>::max_size(allocs.elems_allocator);
            return std::min(a, b);
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

            Allocation new_memory = allocs.allocate(n);
            aul::uninitialized_move(mem.index_array, mem.index_array + mem.size, new_memory.index_array, allocs.index_allocator);
            aul::uninitialized_move(mem.elems_array, mem.elems_array + mem.size, new_memory.elems_array, allocs.elems_allocator);
            aul::uninitialized_move(mem.erase_array, mem.erase_array + mem.size, new_memory.erase_array, allocs.erase_allocator);

            //Create new empty
            aul::uninitialized_iota(
                new_memory.index_array + mem.capacity,
                new_memory.index_array + new_memory.capacity,
                mem.capacity + 1,
                allocs.index_allocator
            );

            if (mem.free_head) {
                //Append previous list head to end of free list
                new_memory.index_array[new_memory.capacity - 1].data() = mem.free_head - mem.index_array;
            } else {
                //Terminate free list
                new_memory.index_array[new_memory.capacity - 1].data() = new_memory.capacity - 1;
            }
            new_memory.size = mem.size;
            new_memory.free_head = new_memory.index_array + mem.capacity;

            allocs.deallocate(this->mem);

            this->mem = std::move(new_memory);
        }

        //TODO: Implement
        void resize(const size_type n, const_reference val);

        //TODO: Implement
        void resize(const size_type n);

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
        /// \param it Iterator to element within this map
        /// \return   key corresponding to element indicates by it
        ///
        [[nodiscard]]
        key_type id_of(const_iterator it) {
            const_pointer p = it.operator->();

            return {
                mem.erase_array[p - mem.elems_array],
                mem.index_array[p - mem.elems_array].version()
            };
        }

        ///
        /// \param x Key to be checked
        /// \return  Returns true if the key maps to a valid element
        ///
        [[nodiscard]]
        inline bool validate_id(key_type x) noexcept {
            return 
                x.version() == mem.index_array[x.data()].version() &&
                x.data() <= size();
        }

        ///
        /// \return Copy of internal allocator
        ///
        [[nodiscard]]
        inline Alloc get_allocator() const noexcept {
            return allocs.elems_allocator;
        }

        ///
        /// \return Pointer to array containing elements
        ///
        [[nodiscard]]
        inline pointer data() noexcept {
            return mem.elems_array;
        }

        ///
        /// \return Pointer to array containing elements
        ///
        [[nodiscard]]
        inline const_pointer data() const noexcept {
            return mem.elems_array;
        }

    private:
        //=====================================================================
        // Instance members
        //=====================================================================

        Allocators allocs;

        Allocation mem;

        //=====================================================================
        // Helper functions
        //=====================================================================

        ///
        /// 
        ///
        void grow(size_type n) {
            constexpr size_type size_type_max = std::numeric_limits<size_type>::max();
            const size_type double_size = (size_type_max / 2) < size() ? size_type_max : 2 * size();

            reserve(std::max(n, double_size));
        }

        ///
        /// \
        ///
        inline index_pointer consume_index(const size_type pos) noexcept {
            const index_pointer free_ptr = mem.free_head;

            if (free_ptr->data() == (free_ptr - mem.index_array) ) {
                mem.free_head = nullptr;
            } else {
                mem.free_head = mem.index_array + (free_ptr->data());
            }

            free_ptr->data() = pos;

            return free_ptr;
        }

        ///
        /// Frees index pointed to by ptr and pushes it onto list of free
        /// indices. Increments index version
        ///
        inline index_pointer release_index(const index_pointer ptr) noexcept {
            if (mem.free_head) {
                *ptr = mem.free_head - mem.index_array;
            } else {
                *ptr = ptr - mem.index_array;
            }
            mem.free_head = ptr;
        }

        inline index_type& index_of(const_pointer ptr) const noexcept {
            return mem.index_array[mem.erase_array[ptr - mem.elems_array]];
        }

        /// Destroys the element pointed to by p through the allocator and
        /// clears the associated inde value.
        /// \param p Pointer to element to be destroyed.
        ///
        void destroy_element(pointer p);

        /// Constructs an element at the specified position along with the
        /// corresponding erase value and index
        ///
        /// \pre mem.free_head points to an already constructed index object
        ///
        /// \tparam     Args args type
        /// \param p    Pointer to desired construction point
        /// \param args Constructor parameters
        template<class... Args>
        inline void emplace_element(pointer pos, Args&& ... args) {
            index_pointer index = mem.free_head;

            consume_free_head(pos - mem.elems_array);

            elems_allocator_traits::construct(
                allocs.elems_allocator,
                pos,
                std::forward<Args>(args)...
            );

            erase_allocator_traits::construct(
                allocs.erase_allocator,
                mem.erase_array + (pos - mem.elems_array),
                index - mem.index_array
            );
        }

        /// Move constructs an element within the container from it's current
        /// position to dest, and updates the associated index and erase
        /// values. Assumes that dest does not point to a position in data[]
        /// that is currently being used.
        ///
        /// \param from Pointer to element to move construct from
        /// \param dest Pointer to desired move-construction point
        ///
        void move_construct_element(pointer from, pointer dest);

        /// Move assigns an element within the container from it's current
        /// position to dest, updates its index, and updates the erase value.
        /// Assumes that dest points to a position in data[] that is currently
        /// being used by an element. 
        ///
        void move_assign_element(pointer from, pointer dest);

        ///
        /// Swaps the position of two elements along with their erase values
        ///
        void swap_elements(pointer a, pointer b);

        //=====================================================================
        // Helper classes
        //=====================================================================

        ///
        /// Wrapper class around all memory allocators needed by Slot_map
        /// class. Abstracts away need for handling copy and move semantics
        /// as well as constructions of allocators individually. 
        ///
        struct Allocators {
            elems_allocator_type elems_allocator;
            index_allocator_type index_allocator;
            erase_allocator_type erase_allocator;

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

            Allocators& operator=(const Allocators& allocs) {
                if constexpr (elems_allocator_traits::propagate_on_container_copy_assignment::value) {
                    this->elems_allocator = allocs.elems_allocator;
                }

                if constexpr (index_allocator_traits::propagate_on_container_copy_assignment::value) {
                    this->index_allocator = allocs.index_allocator;
                }

                if constexpr (erase_allocator_traits::propagate_on_container_copy_assignment::value) {
                    this->erase_allocator = allocs.erase_allocator;
                }
            }

            Allocators& operator=(Allocators&& allocs) {
                if constexpr (elems_allocator_traits::propagate_on_container_move_assignment::value) {
                    this->elems_allocator = std::move(allocs.elems_allocator);
                }

                if constexpr (index_allocator_traits::propagate_on_container_move_assignment::value) {
                    this->index_allocator = std::move(allocs.index_allocator);
                }

                if constexpr (erase_allocator_traits::propagate_on_container_move_assignment::value) {
                    this->erase_allocator = std::move(allocs.erase_allocator);
                }
            }

            void swap(Allocators& allocs) {
                if constexpr (elems_allocator_traits::propagate_on_container_swap::value) {
                    std::swap(this->elems_allocator, allocs.elems_allocator);
                }

                if constexpr (index_allocator_traits::propagate_on_container_swap::value) {
                    std::swap(this->index_allocator, allocs.index_allocator);
                }

                if constexpr (erase_allocator_traits::propagate_on_container_swap::value) {
                    std::swap(this->erase_allocator, allocs.erase_allocator);
                }
            }

            ~Allocators() = default;

            inline Allocation allocate(const size_type count) {
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

            inline Allocation& allocate(size_type count, Allocation& mem_hint) {
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

            inline void deallocate(Allocation& mem) {
                try {
                    index_allocator_traits::deallocate(index_allocator, mem.index_array, mem.capacity);
                    elems_allocator_traits::deallocate(elems_allocator, mem.elems_array, mem.capacity);
                    erase_allocator_traits::deallocate(erase_allocator, mem.erase_array, mem.capacity);
                    mem.clear();

                } catch (...) {
                    throw;
                }
            }

        };

        struct Allocation {

            //=================================================================
            // Instance variables
            //=================================================================

            index_pointer index_array;
            elems_pointer elems_array;
            erase_pointer erase_array;

            index_pointer free_head;

            size_type capacity;
            size_type size;

            //=================================================================
            // -ctors
            //=================================================================

            Allocation() = default;
            Allocation(const Allocation&) = default;
            Allocation(Allocation&&) = default;
            ~Allocation() = default;

            //=================================================================
            // Assignment operators
            //=================================================================

            Allocation& operator=(const Allocation&) = default;

            Allocation& operator=(Allocation&& alloc) {
                index_array = std::move(alloc.index_array);
                elems_array = std::move(alloc.elems_array);
                erase_array = std::move(alloc.erase_array);

                free_head = std::move(alloc.free_head);
                capacity = std::move(alloc.capacity);
                size = std::move(alloc.size);

                alloc.clear();

                return *this;
            }

            //=================================================================
            // Misc. methods
            //=================================================================

            ///
            /// Resets all pointers to nullptr and integral values to 0
            ///
            inline void clear() noexcept {
                index_array = nullptr;
                elems_array = nullptr;
                erase_array = nullptr;

                free_head = nullptr;

                size = 0;
                capacity = 0;
            }

        };

    };// class aul::Slot_map<T, Alloc>

}// namespace aul

#endif
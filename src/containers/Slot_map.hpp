#ifndef AUL_SLOT_MAP_HPP
#define AUL_SLOT_MAP_HPP

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#include "../Versioned_type.hpp"
#include "../memory/Memory.hpp"
#include <memory>
#include <utility>
#include <initializer_list>
#include <numeric>
#include <type_traits>
#include <algorithm>
#include <limits.h>
#include <string>

#include <iostream>
#include <iomanip>

namespace aul {

    /*	Slot_map_iterator is a contiguous iterator for the Slot_map class
    */
    template<class Allocator_types, bool is_const>
    class Slot_map_iterator {
    public:
        using value_type = typename Allocator_types::value_type;
        using difference_type = typename Allocator_types::difference_type;
        using pointer = typename std::conditional<is_const,
            typename Allocator_types::const_pointer,
            typename Allocator_types::pointer
        >::type;
        using reference = typename std::conditional<is_const, const value_type&, value_type&>::type;
        using iterator_category = std::random_access_iterator_tag;

        //-------------------------
        // Constructors
        //-------------------------

        Slot_map_iterator(pointer x = pointer{})
            : pos{x} {}

        Slot_map_iterator(Slot_map_iterator& it)
            : pos{it.pos} {}

        Slot_map_iterator(Slot_map_iterator&& it)
            : pos{it.pos} {
            it.pos = pointer{};
        }

    public:

        //-------------------------
        // Assignment operators
        //-------------------------

        Slot_map_iterator& operator=(const Slot_map_iterator& x) {
            pos = x.pos;
        }

        Slot_map_iterator& operator=(Slot_map_iterator&& x) {
            pos = x.pos;
            x.pos = pointer{};
        }

        Slot_map_iterator& operator+=(const difference_type x) {
            pos += x;
            return *this;
        }

        Slot_map_iterator& operator-=(const difference_type x) {
            pos -= x;
            return *this;
        }

        //-------------------------
        // Comparision operators
        //-------------------------

        inline bool operator==(const Slot_map_iterator x) const {
            return this->pos == x.pos;
        }

        inline bool operator!=(const Slot_map_iterator x) const {
            return this->pos != x.pos;
        }

        inline bool operator<(const Slot_map_iterator x) const {
            return this->pos < x.pos;
        }

        inline bool operator>(const Slot_map_iterator x) const {
            return this->pos > x.pos;
        }

        inline bool operator<=(const Slot_map_iterator x) {
            return !(this->pos > x, pos);
        }

        inline bool operator>=(const Slot_map_iterator x) {
            return !(this->pos < x.pos);
        }

        //-------------------------
        // In-/Decrement operators
        //-------------------------

        Slot_map_iterator& operator++() {
            ++pos;
            return *this;
        }

        Slot_map_iterator& operator++(int) {
            auto temp = *this;
            ++pos;
            return temp;
        }

        Slot_map_iterator& operator--() {
            --pos;
            return *this;
        }

        Slot_map_iterator& operator--(int) {
            auto temp = *this;
            --pos;
            return temp;
        }

        //-------------------------
        // Arithmetic operators
        //-------------------------

        Slot_map_iterator operator+(difference_type x) const {
            return Slot_map_iterator(pos + x);
        }

        friend Slot_map_iterator operator+(difference_type lh, Slot_map_iterator rh) {
            return rh += lh;
        }

        Slot_map_iterator operator-(difference_type x) const {
            return Slot_map_iterator(pos - x);
        }

        friend Slot_map_iterator operator-(difference_type lh, Slot_map_iterator rh) {
            return rh -= lh;
        }

        //-------------------------
        // Dereference operators
        //-------------------------

        reference operator*() {
            return *pos;
        }

        pointer operator->() {
            return pos;
        }

        operator Slot_map_iterator<Allocator_types, true>() {
            return Slot_map_iterator<Allocator_types, true>(pos);
        }

    private:
        pointer pos;

    }; //end Slot_map_iterator<is_const>




    /// A vector like associative container which offers constant time look-up,
    /// insertion, and deletion.
    ///
    template<typename T, class Alloc = std::allocator<T>>
    class Slot_map {
    private:

        //TODO: Implemention of C++20 ranges

        //-----------------------------
        // Forward declarations
        //-----------------------------

        class Slot_map_base;

    public:

        //=====================================================================
        //	Type aliases
        //=====================================================================

        using size_type = typename std::allocator_traits<Alloc>::size_type;
        using difference_type = typename std::allocator_traits<Alloc>::difference_type;

        using value_type = T;
        using index_type = aul::Versioned_type<T*, size_type>;
        using erase_type = size_type;

        using key_type     = aul::Versioned_type<size_type, size_type>;

        using pointer = typename std::allocator_traits<Alloc>::pointer;//T*;
        using const_pointer = typename std::allocator_traits<Alloc>::const_pointer;// const T*;

        using reference = T&;
        using const_reference = const T&;

        using allocator_type = Alloc;

        using iterator = Slot_map_iterator<typename aul::Allocator_types<Alloc>, false>;
        using const_iterator = Slot_map_iterator<typename aul::Allocator_types<Alloc>, true>;

        using reverse_iterator = typename std::reverse_iterator<iterator>;
        using const_reverse_iterator = typename std::reverse_iterator<const_iterator>;

        //=====================================================================
        //	Constructors
        //=====================================================================

        ///	Default Contructor
        /// Constructs an empty instance of Slot_map with default constructed
        /// allocators.
        Slot_map() noexcept(noexcept(Alloc()))
            : Slot_map(Alloc()) {}

        ///	Allocator constructor
        /// Constructs an empty Slot_map that uses copies of Alloc.
        explicit Slot_map(const Alloc& alloc) noexcept
            : base(0, alloc) {}

        ///	Fill contructor
        /// Constructs a Slot_map with n copies of value. If Alloc is provided,
        /// Slot_map creates 3 copies to use internally, otherwise, all
        /// allocators are copy constructed from a default
        ///
        Slot_map(const size_type n, const T& value, const Alloc& allocator = {})
            : base(n, allocator) {
            base.data_last = base.data_end;
            base.index_free = base.index_end;
            base.erase_last = base.erase_end;

            aul::uninitialized_fill(base.data_begin, base.data_end, value, base.data_allocator);
            aul::uninitialized_iota(base.index_begin, base.index_end, base.data_begin, base.index_allocator);
            aul::uninitialized_iota(base.erase_begin, base.erase_end, 0u, base.erase_allocator);
        }

        /// Reserves enough space for n elements creates an internal allocator
        /// by copying constructing from alloc
        ///
        Slot_map(const size_type n, const Alloc& alloc = {})
            : base(n, alloc) {
            aul::uninitialized_fill(base.index_begin, base.index_end, index_type(), base.index_allocator);
            aul::uninitialized_iota(base.erase_begin, base.erase_end, 0u, base.erase_allocator);
        }

        /// Constructs a new Slot_map from the elements within the range
        /// specified by the
        /// iterators begin and end
        template<class InputIter>
        Slot_map(InputIter begin, InputIter end, const Alloc& allocator = {})
            :base(end - begin, allocator) {
            base.data_last = base.data_end;
            base.index_free = base.index_end;
            base.erase_last = base.erase_end;

            aul::uninitialized_copy(begin, end, base.data_begin, base.data_allocator);
            aul::uninitialized_iota(base.index_begin, base.index_end, base.data_begin, base.index_allocator);
            aul::uninitialized_iota(base.erase_begin, base.erase_end, 0, base.erase_allocator);
        }

        /// Constructs a Slot_map with elements copy_constructed from list. The
        /// internal allocatator is copy-constructed from allocator.
        ///
        ///
        Slot_map(const std::initializer_list<T> list, Alloc allocator = {})
            : Slot_map{
            list.begin(),
            list.end(),
            allocator} {}

        //Move contructor
        Slot_map(Slot_map&& right) noexcept
            : base{std::move(right.base)} {
            static_assert(std::is_move_constructible<T>::value, "Type T is not move constructible.");
        }

        //Move contructor with allocator
        Slot_map(Slot_map&& right, Alloc alloc) noexcept
            : base{
            std::move(right.base),
            alloc} {
            static_assert(std::is_move_constructible<T>::value, "Type T is not move constructible.");
        }

        //Copy Constructor
        Slot_map(const Slot_map& right)
            : base{right.base} {
            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy constructible.");

            base.data_last = base.data_begin + right.size();
            base.index_free = base.index_begin + (right.base.index_free - right.base.index_begin);
            base.erase_last = base.erase_begin + right.size();

            aul::uninitialized_copy(right.base.data_begin, right.base.data_end, base.data_begin, base.data_allocator);
            aul::uninitialized_copy(right.base.index_begin, right.base.index_end, base.index_begin,
                                    right.base.index_allocator);
            aul::uninitialized_copy(right.base.erase_begin, right.base.erase_end, base.erase_begin,
                                    base.erase_allocator);
        }

        //Copy Constructor with allocator
        Slot_map(const Slot_map& right, Alloc alloc)
            : base{
            right.base,
            alloc} {
            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy constructible.");

            base.data_last = base.data_begin + right.size();
            base.index_free = base.index_begin + (right.base.index_free - right.base.index_begin);
            base.erase_last = base.erase_begin + right.size();

            aul::uninitialized_copy(right.base.data_begin, right.base.data_end, base.data_begin, base.data_allocator);
            aul::uninitialized_copy(right.base.index_begin, right.base.index_end, base.index_begin,
                                    right.base.index_allocator);
            aul::uninitialized_copy(right.base.erase_begin, right.base.erase_end, base.erase_begin,
                                    base.erase_allocator);

            offset_indicies(right.base.data_begin, base.data_begin);
        }

        //=====================================================================
        //	Destructor
        //=====================================================================

        ~Slot_map() {
            clear();
        }

        //=====================================================================
        //	Misc member methods
        //=====================================================================

        void assign(const std::initializer_list<T> list) {
            assign(list.begin(), list.end());
        }

        template<typename Iter>
        void assign(Iter begin, Iter end) {
            clear();
            reserve(end - begin);

            base.data_last = base.data_begin + (end - begin);
            base.index_free = base.index_begin + (end - begin);
            base.erase_last = base.erase_begin + (end - begin);

            aul::uninitialized_copy(begin, end, base.data_begin, base.data_allocator);
            aul::uninitialized_iota(base.index_begin, base.index_free, base.data_begin, base.index_allocator);
            aul::uninitialized_iota(base.erase_begin, base.erase_last, 0, base.erase_allocator);
        }

        void assign(const size_type n, const T& val) {
            clear();
            reserve(n);

            base.data_last = base.data_begin + n;
            base.index_free = base.index_begin + n;
            base.erase_last = base.erase_begin + n;

            aul::uninitialized_fill(base.data_begin, base.data_begin + n, val, base.data_allocator);
            aul::uninitialized_iota(base.index_begin, base.index_free, base.data_begin, base.index_allocator);
            aul::uninitialized_iota(base.erase_begin, base.erase_last, 0, base.erase_allocator);
        }

        void clear() noexcept {
            //Destroys data
            auto destroyer = [this](index_type i) {
                if (i) {
                    std::allocator_traits<Alloc>::destroy(base.data_allocator, i.data());
                }
            };

            std::for_each(base.index_begin, base.index_end, destroyer);

            //Destroy index/erase values
            aul::destroy(base.index_begin, base.index_end, base.index_allocator);
            aul::destroy(base.erase_begin, base.erase_end, base.erase_allocator);

            //Reset capcity to 0
            base.clear();
        }

        void swap(Slot_map& right) noexcept(
        std::allocator_traits<Alloc>::propagate_on_container_swap::value ||
        std::allocator_traits<Alloc>::is_always_equal::value
        ) {
            std::swap(this->base, right.base);
        }

        void straighten();

        void sort();

        void stable_sort();

        //=====================================================================
        //	Assignment methods/operators
        //=====================================================================

        Slot_map& operator=(Slot_map& right) {
            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy constructible.");

            if (this == &right) {
                return *this;
            }

            clear();
            base = right.base;

            //Copy contruct data in this->base
            aul::uninitialized_copy(right.base.data_begin, right.base.data_end, base.data_begin, base.data_allocator);
            aul::uninitialized_copy(right.base.index_begin, right.base.index_end, base.index_begin,
                                    base.index_allocator);
            aul::uninitialized_copy(right.base.erase_begin, right.base.erase_end, base.erase_begin,
                                    base.erase_allocator);

            //Add offset to pointers in indicies to point to new data[]
            offset_indicies(right.base.data_begin, base.data_begin);

            //Update pointers
            base.data_last = base.data_begin + right.size();
            base.index_free = base.index_begin + (right.base.index_free - right.base.index_begin);
            base.erase_last = base.erase_begin + right.size();
        }

        Slot_map& operator=(Slot_map&& right) noexcept {
            if (this == &right) {
                return *this;
            }

            base = std::move(right.base);
            return *this;
        }

        Slot_map& operator=(const std::initializer_list<T> list) {
            clear();
            reserve(list.size());

            //Copy contruct data and default contruct index/erase values
            aul::uninitialized_copy(list.begin(), list.end(), base.data_begin, base.data_allocator);
            aul::uninitialized_iota(base.index_begin, base.index_end, base.data_begin, base.index_allocator);
            aul::uninitialized_iota(base.erase_begin, base.erase_end, 0, base.erase_allocator);

            base.data_last = base.data_begin + list.size();
            base.index_free = base.index_begin + list.size();
            base.erase_last = base.erase_begin + list.size();
        }

        //=====================================================================
        //	Access methods
        //=====================================================================

        reference front() {
            return *base.data_begin;
        }

        const_reference front() const {
            return *base.data_begin;
        }

        reference back() {
            return *(base.data_last);
        }

        const_reference back() const {
            return *(base.data_last);
        }

        key_type id_of(const_iterator it) {
            const_pointer p = it.operator->();

            return {
                base.erase_begin[p - base.data_begin],
                base.index_begin[p - base.data_begin].version()
            };
        }

        bool validate_id(key_type x) {
            return
                (x.version() == base.index_begin[x.data()].version()) &&
                (x.data() <= size());
        }

        reference at(const size_type x) {
            if (size() < x) {
                throw std::out_of_range("Index out of bounds.");
            }
            return base.data_begin[x];
        }

        const_reference at(const size_type x) const {
            if (size() < x) {
                throw std::out_of_range("Index out of bounds.");
            }
            return base.data_begin[x];
        }

        reference at(const key_type x) {
            if (size() < x.data()) {
                throw std::out_of_range("Index out of bounds.");
            }

            index_type index = base.index_begin[x.data()];

            if (x.version() != index.version()) {
                throw std::invalid_argument("Version mismatch.");
            }

            return *(index.data());
        }

        const_reference at(const key_type x) const {
            if (size() < x.data()) {
                throw std::out_of_range("Index out of bounds.");
            }

            index_type index = base.index_begin[x.data()];

            if (x.version() != index.version()) {
                throw std::invalid_argument("Version mismatch.");
            }

            return *(index.data());
        }

        //=====================================================================
        //	Access operators
        //=====================================================================

        reference operator[](const size_type x) {
            return base.data_begin[x];
        }

        const_reference operator[](const size_type x) const {
            return base.data_begin[x];
        }

        reference operator[](const key_type x) {
            index_type index = base.index_begin[x.data()];
            return *(index.data());
        }

        const_reference operator[](const key_type x) const {
            index_type index = base.index_begin[x.data()];
            return *(index.data());
        }

        //=====================================================================
        //	Modification methods
        //=====================================================================

        iterator insert(const_iterator it, const T& val) {
            emplace(it, val);
            return iterator(const_cast<pointer>(it.operator->()));
        }

        iterator insert(const_iterator it, T&& val) {
            emplace(it, std::move(val));
            return iterator(const_cast<pointer>(it.operator->()));
        }

        iterator stable_insert(const_iterator it, const T& val) {

        }

        iterator stable_insert(const_iterator it, T&& val) {

        }

        iterator insert(const_iterator it, const size_type n, const T& val);

        /*
        {
            size_type pos_index = it.pos - base.data_begin;

            grow(size() + n);
            //TODO: Implement

            //Number of existing elements to be moved right
            size_type move_count = std::max(0, base.data_last - it);

            for (int i = 0; i != n; ++i) {
                move_construct_element(base.data_last - 1 - i, base.data_last - 1 - i + n);
            }

            return iterator(nullptr);
        }

        template<class InputIt>
        iterator insert(const_iterator it, InputIt begin, InputIt end) {
            //TODO: Implement

            return iterator(nullptr);
        }

        iterator insert(const_iterator it, std::initializer_list<T> list) {
            return insert(it, list.begin(), list.end());
        }
        */

        void erase(const key_type id) {
            if (!validate_id(id)) {
                throw std::invalid_argument("Version mismatch.");
            }

            size_type pos = base.index_begin[id.data()].data() - base.data_begin;
            //erase(begin() + pos);
        }

        /*
        */
        void erase(const_iterator x) {
            index_type* index = index_of(x.operator->());
            std::cout << index << std::endl;

            if (x.operator->() != (base.data_last - 1)) {
                swap_elements(const_cast<pointer>(x.operator->()), base.data_last - 1);
            }
            destruct_element(base.data_last - 1);

            find_free_index(index);
            --base.data_last;
            --base.erase_last;
        }

        void push_back(const T& val) {
            emplace_back(val);
        }

        void push_back(T&& val) {
            emplace_back(std::move(val));
        }

        void pop_back() {
            index_type* hint = index_of(base.data_last - 1);

            destruct_element(base.data_last - 1);

            --base.data_last;
            --base.erase_last;
            find_free_index(hint);
        }

        template<class... Args>
        reference emplace_back(Args&& ... args) {
            grow(size() + 1);

            emplace_element(base.data_last, std::forward<Args>(args)...);

            find_free_index();
            ++base.data_last;
            ++base.erase_last;

            return base.data_last[-1];
        }

        template<class...Args>
        reference emplace(const_iterator pos, Args&& ... args) {
            size_type position = pos.pos - base.data_begin;

            grow(size() + 1);

            pointer p = base.data_begin + position;

            //Conditionally move pointed to element to last;
            if (p != base.data_last) {
                move_construct_element(p, base.data_last);
            }

            //Construct element at p
            emplace_element(p, std::forward<Args>(args)...);
            std::cout << *p << std::endl;

            find_free_index();

            ++base.data_last;
            ++base.erase_last;

            return base.data_last[-1];
        }


        template<class...Args>
        reference stable_emplace(const_iterator pos, Args&& ... args) {
            size_type position = pos.pos - base.data_begin;
        }


        //=====================================================================
        //	Iterator methods
        //=====================================================================

        iterator begin() noexcept {
            return iterator(base.data_begin);
        }

        const_iterator begin() const noexcept {
            return const_iterator(base.data_begin);
        }

        const_iterator cbegin() const noexcept {
            return const_iterator(base.data_begin);
        }


        iterator end() noexcept {
            return iterator(base.data_last);
        }

        const_iterator end() const noexcept {
            return const_iterator(base.data_last);
        }

        const_iterator cend() const noexcept {
            return const_iterator(base.data_last);
        }


        reverse_iterator rbegin() noexcept {
            return reverse_iterator(base.data_last - 1);
        }

        const_reverse_iterator rbegin() const noexcept {
            return reverse_iterator(base.data_last - 1);
        }

        const_reverse_iterator crbegin() const noexcept {
            return const_reverse_iterator(base.data_last - 1);
        }


        reverse_iterator rend() noexcept {
            return reverse_iterator(base.data_begin);
        }

        const_reverse_iterator rend() const noexcept {
            return reverse_iterator(base.data_begin);
        }

        const_reverse_iterator crend() const noexcept {
            return const_reverse_iterator(base.data_begin);
        }

        //=====================================================================
        //	Capacity/size methods
        //=====================================================================

        [[nodiscard]] bool empty() const noexcept {
            return base.data_begin == base.data_end;
        }

        [[nodiscard]] size_type size() const noexcept {
            return base.data_last - base.data_begin;
        }

        [[nodiscard]] size_type max_size() const noexcept {
            constexpr size_type a = std::numeric_limits<difference_type>::max();
            size_type b = std::allocator_traits<Alloc>::max_size(base.data_allocator);
            return std::min(a, b);
        }

        inline void reserve(const size_type n) {
            //Return if new capacity is smaller or greater than current
            if (n <= capacity()) {
                return;
            }

            //Ensure Slot_map can grow
            if (max_size() < n) {
                throw std::length_error("Slot_map cannot grow.");
            }

            //Store element count and previous capcity
            size_type count = size();
            size_type old_capacity = capacity();

            //Pointer to newly allo0cated memory
            value_type* data_begin = nullptr;
            index_type* index_begin = nullptr;
            erase_type* erase_begin = nullptr;


            //Allocate new memory
            try {
                data_begin = base.data_allocator.allocate(n);
                index_begin = base.index_allocator.allocate(n);
                erase_begin = base.erase_allocator.allocate(n);
            } catch (...) {
                //Deallocate all memory if allocation fails
                base.data_allocator.deallocate(data_begin, n);
                base.index_allocator.deallocate(index_begin, n);
                base.erase_allocator.deallocate(erase_begin, n);
                throw;
            }

            //Store the differences between the old and new locations in memory
            //difference_type data_offset  = data_begin - base.data_begin;
            //difference_type index_offset = index_begin - base.index_begin;
            //difference_type erase_offset = erase_begin - base.erase_begin;

            //Update index pointers
            offset_indicies(base.data_begin, data_begin);

            //Move data to new arrays.
            aul::uninitialized_move(base.data_begin, base.data_end, data_begin, base.data_allocator);
            aul::uninitialized_move(base.index_begin, base.index_end, index_begin, base.index_allocator);
            aul::uninitialized_move(base.erase_begin, base.erase_end, erase_begin, base.erase_allocator);

            //create new indicies to fill empty space
            aul::uninitialized_fill(index_begin + old_capacity, index_begin + n, index_type(), base.index_allocator);

            base.data_begin = data_begin;
            base.data_last = base.data_begin + count;
            base.data_end = base.data_begin + n;

            base.index_free = index_begin + (base.index_free - base.index_begin);
            base.index_begin = index_begin;
            base.index_end = index_begin + n;

            base.erase_begin = erase_begin;
            base.erase_last = erase_begin + count;
            base.erase_end = erase_begin + n;
        }

        size_type capacity() const noexcept {
            return base.data_end - base.data_begin;
        }

        /*	Reducing capacity to be equal to size(). Potentially invalidates all
            IDs.
        */
        void shrink_to_fit() {
            //TODO: Ask allocator?
        }

        void resize(const size_type n, const_reference val) {
            //TODO:
        }

        void resize(const size_type n) {
            if (size() < n) {
                reserve(n);
            }

            //TODO
        }

        void set_growth_factor(const float fac) {
            if (fac <= 1.0f) {
                growth_factor = 1.1;
            } else {
                growth_factor = fac;
            }
        }

        //=====================================================================
        //	Operators
        //=====================================================================

        friend bool operator==(const Slot_map& lh, const Slot_map& rh) {
            if (lh.size() != rh.size()) {
                return false;
            }

            for (size_type i = 0; i < lh.size(); ++i) {
                if (lh[i] != rh[i]) {
                    return false;
                }
            }

            return true;
        }

        friend bool operator!=(const Slot_map& lh, const Slot_map& rh) {
            return !operator==(lh, rh);
        }

        friend bool operator<(const Slot_map& lh, const Slot_map& rh) {
            for (size_type i = 0; i < std::min(lh.size(), rh.size()); ++i) {
                if (lh[i] < rh[i]) {
                    return true;
                }
            }

            return lh.size() < rh.size();
        }

        friend bool operator>(const Slot_map& lh, const Slot_map& rh) {
            for (size_type i = 0; i < std::min(lh.size(), rh.size()); ++i) {
                if (lh[i] > rh[i]) {
                    return true;
                }
            }

            return lh.size() > rh.size();
        }

        friend bool operator<=(const Slot_map& lh, const Slot_map& rh) {
            return !operator>(lh, rh);
        }

        friend bool operator>=(const Slot_map& lh, const Slot_map& rh) {
            return !operator<(lh, rh);
        }

        //Internal access methods
        inline pointer data() noexcept {
            return base.data_begin;
        }

        inline const_pointer data() const noexcept {
            return base.data_begin;
        }

        inline index_type* indicies() noexcept {
            return base.index_begin;
        }

        inline const index_type* indicies() const noexcept {
            return base.index_begin;
        }

        inline erase_type* erase_table() noexcept {
            return base.erase_begin;
        }

        inline const erase_type* erase_table() const noexcept {
            return base.erase_begin;
        }

        //Allocator methods

        inline Alloc get_allocator() const noexcept {
            return base.data_allocator;
        }

        //Debugging methods

        void print() const {
            using std::cout;
            using std::endl;

            difference_type free_index_index = base.index_free - base.index_begin;
            difference_type data_last_index = base.data_last - base.data_begin;
            difference_type erase_last_index = base.erase_last - base.erase_begin;

            std::string partition = std::string(60, '=');

            cout << endl;
            cout << partition << endl;
            cout << "Slot_map using " << size() << '/' << capacity() << " slots." << endl;
            cout << "Last data  at " << data_last_index << '.' << endl;
            cout << "Free index at " << free_index_index << '.' << endl;
            cout << "Last erase at " << erase_last_index << '.' << endl;

            cout << endl;

            cout << "Data\t" << base.data_begin << '\t' << base.data_last << '\t' << base.data_end << endl;
            cout << "Index\t" << base.index_begin << '\t' << base.index_free << '\t' << base.index_end << endl;
            cout << "Erase\t" << base.erase_begin << '\t' << base.erase_last << '\t' << base.erase_end << endl;

            cout << endl;

            std::unique_ptr<bool[]> marked{new bool[capacity()]};

            for (unsigned i = 0; i < capacity(); ++i) {
                marked[i] = base.index_begin[i].data();
            }

            for (unsigned i = 0; i < capacity(); ++i) {
                cout << i << '\t';
                cout << std::setfill('0');
                cout << '(' << std::setw(9) << base.index_begin[i].data() << ", " << base.index_begin[i].version()
                     << ")\t";

                if (marked[i]) {
                    cout << base.data_begin + i;
                } else {
                    cout << std::right << std::setw(9);
                    cout << (void*) nullptr;
                }

                cout << '\t';
                cout << base.erase_begin[i];

                cout << endl;
            }
            cout << partition << endl;
            cout << endl;
        }

        void print_elements() {
            using std::cout;
            using std::endl;

            cout << "Elements:" << endl;
            for (pointer p = base.data_begin; p != base.data_last; ++p) {
                cout << '[' << p << "] " << *p << endl;
            }
        }

    private:
        //=====================================================================
        //	Instance members
        //=====================================================================

        Slot_map_base base;
        float growth_factor = 2.0f;

        //=====================================================================
        //	Helper functions
        //=====================================================================

        inline index_type* index_of(const_pointer x) {
            return base.index_begin + (base.erase_begin[x - base.data_begin]);
        }

        /*	Offset all indicies in the current Slot_map to be realtive to the
            location of the new_begin pointer.
        */
        void offset_indicies(pointer old_begin, pointer new_begin) {
            auto offsetter = [old_begin, new_begin](index_type& i) {
                if (i) {
                    i.data() = new_begin + (i.data() - old_begin);
                }
            };

            std::for_each(base.index_begin, base.index_end, offsetter);
        }

        /*	Increases the capacity of the current intance by multiplying the
            current capacity by the growth factor until it at least matches
            the target size.
        */
        void grow(const size_type target) {
            size_type new_capacity;

            if (capacity() == 0) {
                new_capacity = 1;
            } else {
                new_capacity = capacity();
            }

            while (new_capacity < target) {
                new_capacity = static_cast<size_type>(new_capacity * growth_factor);
            }

            reserve(new_capacity);
        }

        void destruct_element(pointer p) {
            std::allocator_traits<Alloc>::destroy(base.data_allocator, p);
            *index_of(p) = nullptr;
        }

        /*	Constructs an element at the specified position along with the
            corresponding erase value and index
        */
        template<class...Args>
        void emplace_element(pointer p, Args&& ... args) {
            std::allocator_traits<Alloc>::construct(base.data_allocator, p, std::forward<Args>(args)...);

            base.index_free->data() = base.data_last;
            *base.erase_last = base.index_free - base.index_begin;
        }

        /*	Move constructs an element within the container from it's current
            position to dest, updates its index, and upates the erase value.
            Assumes that dest does not point to a position in data[] that is
            currently being used.
        */
        void move_construct_element(pointer from, pointer dest) {
            //Move construct at dest
            std::allocator_traits<Alloc>::construct(base.data_allocator, dest, std::move(*from));

            //Use free index to store new address
            base.index_free->data() = dest;

            //Assign erase index for newly created element
            base.erase_begin[base.index_free - base.index_begin] = dest - base.data_begin;
        }

        /*	Move assigns an element within the container from it's current
            position to dest, updates its index, and upates the erase value.
            Assumes that dest points to a position in data[] that is currently
            being used by an element.
        */
        void move_assign_element(pointer from, pointer dest) {
            erase_type* from_erase = base.erase_begin + (base.data_begin - from);
            erase_type* dest_erase = base.erase_begin + (base.data_begin - dest);

            index_type* from_index = base.index_begin + *from_erase;
            index_type* dest_index = base.index_begin + *dest_erase;

            dest_index->data() = from_index->data();
            *dest_erase = *from_erase;
            *dest = *from;
        }

        /*	Swaps the position of two elements along with their erase vaues
        */
        void swap_elements(pointer a, pointer b) {
            if (a == b) {
                return;
            }

            index_type& aIndex = *index_of(a);
            index_type& bIndex = *index_of(b);

            erase_type& aErase = base.erase_begin[a - base.data_begin];
            erase_type& bErase = base.erase_begin[b - base.data_begin];

            std::swap(aIndex.data(), bIndex.data());
            std::swap(aErase, bErase);
            std::swap(*a, *b);
        }

        /*	Moves index_free forward. The optional suggestion pointer should
            point to a location containing an empty index. If the suggestion is
            nullptr or it does not point to a valid location, moves index_free
            right until an empty index is found.
        */
        index_type& find_free_index(index_type* suggestion = nullptr) {
            //Check if suggestion is valid place for index_free;

            if (suggestion && !suggestion->data()) {
                base.index_free = suggestion;
            } else {
                //Otherwise search through indices until and empty is found or they run out
                while ((base.index_free->data() != nullptr) && (base.index_free < base.index_end)) {
                    ++base.index_free;
                }
            }

            return *base.index_free;
        }





        //=======================================
        //	Helper class
        //=======================================

        class Slot_map_base {
            using data_allocator_type  = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
            using index_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<index_type>;
            using erase_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<erase_type>;

            using data_allocator_traits  = std::allocator_traits<data_allocator_type>;
            using index_allocator_traits = std::allocator_traits<index_allocator_type>;
            using erase_allocator_traits = std::allocator_traits<erase_allocator_type>;

        public:
            index_type* index_begin = nullptr;
            index_type* index_free = nullptr;
            index_type* index_end = nullptr;

            value_type* data_begin = nullptr;
            value_type* data_last = nullptr;
            value_type* data_end = nullptr;

            erase_type* erase_begin = nullptr;
            erase_type* erase_last = nullptr;
            erase_type* erase_end = nullptr;

            data_allocator_type data_allocator;
            index_allocator_type index_allocator;
            erase_allocator_type erase_allocator;

            //Default constructor
            Slot_map_base(size_type n = 0, Alloc alloc = Alloc())
                : data_allocator{alloc},
                  index_allocator{index_allocator_type(alloc)},
                  erase_allocator{erase_allocator_type(alloc)} {
                allocate(n);
            }

            //Copy contructor
            Slot_map_base(const Slot_map_base& src)
                : data_allocator{data_allocator_traits::select_on_container_copy_construction(src.data_allocator)},
                  index_allocator{index_allocator_traits::select_on_container_copy_construction(src.index_allocator)},
                  erase_allocator{erase_allocator_traits::select_on_container_copy_construction(src.erase_allocator)} {
                allocate(src.data_end - src.data_begin);
            }

            //Copy constructor with allocator
            Slot_map_base(const Slot_map_base& src, Alloc alloc)
                : data_allocator{data_allocator_type(alloc)},
                  index_allocator{index_allocator_type(alloc)},
                  erase_allocator{erase_allocator_type(alloc)} {
                allocate(src.data_end - src.data_begin);
            }

            //Move constructor
            Slot_map_base(Slot_map_base&& src)
                : data_allocator{std::move(src.data_allocator)},
                  index_allocator{std::move(src.index_allocator)},
                  erase_allocator{std::move(src.erase_allocator)} {
                data_begin = src.data_begin;
                data_last = src.data_last;
                data_end = src.data_end;

                index_begin = src.index_begin;
                index_free = src.index_free;
                index_end = src.index_end;

                erase_begin = src.erase_begin;
                erase_last = src.erase_last;
                erase_end = src.erase_end;

                src.reset_pointers();
            }

            //Move constructor with allocator
            Slot_map_base(Slot_map_base&& src, Alloc alloc)
                : data_allocator{data_allocator_type(alloc)},
                  index_allocator{index_allocator_type(alloc)},
                  erase_allocator{erase_allocator_type(alloc)} {
                data_begin = src.data_begin;
                data_last = src.data_last;
                data_end = src.data_end;

                index_begin = src.index_begin;
                index_free = src.index_free;
                index_end = src.index_end;

                erase_begin = src.erase_begin;
                erase_last = src.erase_last;
                erase_end = src.erase_end;

                src.reset_pointers();
            }

            //Destructor
            ~Slot_map_base() {
                deallocate(data_end - data_begin);
            }

            //Copy assignment operator
            Slot_map_base& operator=(const Slot_map_base& src) {
                deallocate(data_end - data_begin);

                data_allocator = data_allocator_traits::propagate_on_container_copy_assignment::value
                                 ? src.data_allocator : data_allocator_type();
                index_allocator = index_allocator_traits::propagate_on_container_copy_assignment::value
                                  ? src.index_allocator : index_allocator_type();
                erase_allocator = erase_allocator_traits::propagate_on_container_copy_assignment::value
                                  ? src.erase_allocator : erase_allocator_type();

                allocate(src.data_end - src.data_begin);

                return *this;
            }

            //Move assignment operator
            Slot_map_base& operator=(Slot_map_base&& src) noexcept {
                data_allocator = data_allocator_traits::propagate_on_container_move_assignment::value
                                 ? src.data_allocator : data_allocator_type();
                index_allocator = index_allocator_traits::propagate_on_container_move_assignment::value
                                  ? src.index_allocator : index_allocator_type();
                erase_allocator = erase_allocator_traits::propagate_on_container_move_assignment::value
                                  ? src.erase_allocator : erase_allocator_type();

                data_begin = src.data_begin;
                data_last = src.data_last;
                data_end = src.data_end;

                index_begin = src.index_begin;
                index_free = src.index_free;
                index_end = src.index_end;

                erase_begin = src.erase_begin;
                erase_last = src.erase_last;
                erase_end = src.erase_end;

                src.reset_pointers();

                return *this;
            }

            void clear() {
                deallocate(data_end - data_begin);
                reset_pointers();
            }

        private:

            void allocate(const size_type n) {
                if (n == 0) { return; }

                try {
                    data_begin = data_allocator.allocate(n);
                    index_begin = index_allocator.allocate(n);
                    erase_begin = erase_allocator.allocate(n);

                    data_last = data_begin;
                    index_free = index_begin;
                    erase_last = erase_begin;

                    data_end = data_begin + n;
                    index_end = index_begin + n;
                    erase_end = erase_begin + n;

                } catch (...) {
                    deallocate(n);
                    reset_pointers();

                    throw;
                }
            }

            void deallocate(const size_type n) {
                data_allocator.deallocate(data_begin, n);
                index_allocator.deallocate(index_begin, n);
                erase_allocator.deallocate(erase_begin, n);
            }

            void reset_pointers() {
                data_begin = data_last = data_end = nullptr;
                index_begin = index_free = index_end = nullptr;
                erase_begin = erase_last = erase_end = nullptr;
            }

        }; //End class Slot_map_base


    }; //End class aul::Slot_map<T, Alloc>





    template<class T>
    class Slot_map_ref {
    public:

        //-------------------------------------------------
        // -ctors
        //-------------------------------------------------

        Slot_map_ref() = default;

        ~Slot_map_ref() = default;

        //-------------------------------------------------
        // Conversion operators
        //-------------------------------------------------

        operator bool() const {
            return this->id.data() == -1;
        }

        typename aul::Slot_map<T>::reference operator*() {

        }

    private:

        aul::Versioned_type<
            typename aul::Slot_map<T>::difference_type,
            typename aul::Slot_map<T>::size_type
        > id = {
            -1,
            0};

    };

}

#endif
=======
#ifndef AUL_SLOT_MAP_HPP
#define AUL_SLOT_MAP_HPP

#pragma warning(disable:4996)

#include "../Versioned_type.hpp"
#include "../memory/Memory.hpp"
#include <memory>
#include <utility>
#include <initializer_list>
#include <numeric>
#include <type_traits>
#include <algorithm>
#include <limits.h>
#include <string>

#include <iostream>
#include <iomanip>

namespace aul {

    /*	Slot_map_iterator is a contiguous iterator for the Slot_map class
    */
    template<class Allocator_types, bool is_const>
    class Slot_map_iterator {
    public:
        using value_type = typename Allocator_types::value_type;
        using difference_type = typename Allocator_types::difference_type;
        using pointer = typename std::conditional<is_const,
                    typename Allocator_types::const_pointer,
                    typename Allocator_types::pointer
            >::type;
        using reference = typename std::conditional<is_const, const value_type&, value_type&>::type;
        using iterator_category = std::random_access_iterator_tag;

        //-------------------------
        // Constructors
        //-------------------------

        Slot_map_iterator(pointer x = nullptr)
            :pos {x} {}

        Slot_map_iterator(Slot_map_iterator& it)
            :pos {it.pos} {}

        Slot_map_iterator(Slot_map_iterator&& it)
            :pos {it.pos} {}

    public:

        //-------------------------
        // Assignment operators
        //-------------------------

        Slot_map_iterator& operator=(const Slot_map_iterator& x) {
            pos = x.pos;
        }

        Slot_map_iterator& operator=(Slot_map_iterator&& x) {
            pos = x.pos;
        }

        Slot_map_iterator& operator+=(const difference_type x) {
            pos += x;
            return *this;
        }

        Slot_map_iterator& operator-=(const difference_type x) {
            pos -= x;
            return *this;
        }

        //-------------------------
        // Comparision operators
        //-------------------------

        inline bool operator==(const Slot_map_iterator x) const {
            return this->pos == x.pos;
        }

        inline bool operator!=(const Slot_map_iterator x) const {
            return this->pos != x.pos;
        }

        inline bool operator<(const Slot_map_iterator x) const {
            return this->pos < x.pos;
        }

        inline bool operator>(const Slot_map_iterator x) const {
            return this->pos > x.pos;
        }

        inline bool operator<=(const Slot_map_iterator x) {
            return !(this->pos > x, pos);
        }

        inline bool operator>=(const Slot_map_iterator x) {
            return !(this->pos < x.pos);
        }

        //-------------------------
        // In-/Decrement operators
        //-------------------------

        Slot_map_iterator& operator++() {
            ++pos;
            return *this;
        }

        Slot_map_iterator& operator++(int) {
            auto temp = *this;
            ++pos;
            return temp;
        }

        Slot_map_iterator& operator--() {
            --pos;
            return *this;
        }

        Slot_map_iterator& operator--(int) {
            auto temp = *this;
            --pos;
            return temp;
        }

        //-------------------------
        // Arithmetic operators
        //-------------------------

        Slot_map_iterator operator+(difference_type x) const {
            return Slot_map_iterator(pos + x);
        }

        friend Slot_map_iterator operator+(difference_type lh, Slot_map_iterator rh) {
            return rh += lh;
        }

        Slot_map_iterator operator-(difference_type x) const {
            return Slot_map_iterator(pos - x);
        }

        friend Slot_map_iterator operator-(difference_type lh, Slot_map_iterator rh) {
            return rh -= lh;
        }

        //-------------------------
        // Dereference operators
        //-------------------------

        reference operator*() {
            return *pos;
        }

        pointer operator->() {
            return pos;
        }

        operator Slot_map_iterator<Allocator_types, true>() {
            return Slot_map_iterator<Allocator_types, true>(pos);
        }

    private:
        pointer pos = nullptr;

    }; //end Slot_map_iterator<is_const>




    /*
        Note: Implementation assumes there are no relevant specializations on
        allocator
    */

    template<typename T, class Alloc = std::allocator<T>>
    class Slot_map {
    private:
        //-----------------------------
        // Forward declarations
        //-----------------------------

        class Slot_map_base;

    public:

        //=====================================================================
        //	Type aliases
        //=====================================================================

        using size_type = typename std::allocator_traits<Alloc>::size_type;
        using difference_type = typename std::allocator_traits<Alloc>::difference_type;

        using value_type = T;
        using index_type = aul::Versioned_type<T*, size_type>;
        using erase_type = size_type;

        using key_type 	 = aul::Versioned_type<size_type, size_type>;

        using pointer = typename std::allocator_traits<Alloc>::pointer;//T*;
        using const_pointer = typename std::allocator_traits<Alloc>::const_pointer;// const T*;

        using reference = T&;
        using const_reference = const T&;

        using allocator_type = Alloc;

        using iterator = Slot_map_iterator<typename aul::Allocator_types<Alloc>, false>;
        using const_iterator = Slot_map_iterator<typename aul::Allocator_types<Alloc>, true>;

        using reverse_iterator = typename std::reverse_iterator<iterator>;
        using const_reverse_iterator = typename std::reverse_iterator<const_iterator>;

        //=====================================================================
        //	Constructors
        //=====================================================================

        /*	Default Contructor
            Constructs an empty instance of Slot_map with default constructed
            allocators.
        */
        Slot_map() noexcept(noexcept(Alloc()))
            :Slot_map(Alloc())
        {}

        /*	Allocator constructor
            Constructs an empty Slot_map that uses copies of Alloc.
        */
        explicit Slot_map(const Alloc& alloc) noexcept
            :base(0, alloc)
        {}

        /*	Fill contructor
            Constructs a Slot_map with n copies of value. If Alloc is provided,
            Slot_map creates 3 copies to use internally, otherwise, all
            allocators are copy constructed from a default
        */
        Slot_map(const size_type n, const T& value, const Alloc& allocator = {})
            :base( n, allocator )
        {
            base. data_last = base. data_end;
            base.index_free = base.index_end;
            base.erase_last = base.erase_end;

            aul::uninitialized_fill(base. data_begin, base. data_end, value, base.data_allocator);
            aul::uninitialized_iota(base.index_begin, base.index_end, base.data_begin, base.index_allocator);
            aul::uninitialized_iota(base.erase_begin, base.erase_end, 0u, base.erase_allocator);
        }

        //Reserve constructor
        Slot_map(const size_type n, const Alloc& alloc = {})
            :base(n, alloc)
        {
            aul::uninitialized_fill(base.index_begin, base.index_end, index_type(), base.index_allocator);
            aul::uninitialized_iota(base.erase_begin, base.erase_end, 0u, base.erase_allocator);
        }

        //Iterator contructor
        template<class InputIter>
        Slot_map(InputIter begin, InputIter end, const Alloc& allocator = {})
            :base( end - begin, allocator)
        {
            base.data_last = base.data_end;
            base.index_free = base.index_end;
            base.erase_last = base.erase_end;

            aul::uninitialized_copy(begin, end, base.data_begin, base.data_allocator);
            aul::uninitialized_iota(base.index_begin, base.index_end, base.data_begin, base.index_allocator);
            aul::uninitialized_iota(base.erase_begin, base.erase_end, 0, base.erase_allocator);
        }

        //Initializer list constructor
        Slot_map(const std::initializer_list<T> list, Alloc allocator = {})
            :Slot_map{list.begin(), list.end(), allocator}
        {}

        //Move contructor
        Slot_map(Slot_map&& right) noexcept
            :base{ std::move(right.base) }
        {
            static_assert(std::is_move_constructible<T>::value, "Type T is not move constructible.");
        }

        //Move contructor with allocator
        Slot_map(Slot_map&& right, Alloc alloc) noexcept
            :base{ std::move(right.base), alloc }
        {
            static_assert(std::is_move_constructible<T>::value, "Type T is not move constructible.");
        }

        //Copy Constructor
        Slot_map(const Slot_map& right)
            :base{right.base}
        {
            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy constructible.");

            base.data_last  = base.data_begin  + right.size();
            base.index_free = base.index_begin + (right.base.index_free - right.base.index_begin);
            base.erase_last = base.erase_begin + right.size();

            aul::uninitialized_copy(right.base.data_begin,  right.base.data_end,  base.data_begin,  base.data_allocator);
            aul::uninitialized_copy(right.base.index_begin, right.base.index_end, base.index_begin, right.base.index_allocator);
            aul::uninitialized_copy(right.base.erase_begin, right.base.erase_end, base.erase_begin, base.erase_allocator);
        }

        //Copy Constructor with allocator
        Slot_map(const Slot_map& right, Alloc alloc)
            :base{ right.base, alloc } {
            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy constructible.");

            base.data_last = base.data_begin + right.size();
            base.index_free = base.index_begin + (right.base.index_free - right.base.index_begin);
            base.erase_last = base.erase_begin + right.size();

            aul::uninitialized_copy(right.base.data_begin, right.base.data_end, base.data_begin, base.data_allocator);
            aul::uninitialized_copy(right.base.index_begin, right.base.index_end, base.index_begin, right.base.index_allocator);
            aul::uninitialized_copy(right.base.erase_begin, right.base.erase_end, base.erase_begin, base.erase_allocator);

            offset_indicies(right.base.data_begin, base.data_begin);
        }

        //=====================================================================
        //	Destructor
        //=====================================================================

        ~Slot_map() {
            clear();
        }

        //=====================================================================
        //	Misc member methods
        //=====================================================================

        void assign(const std::initializer_list<T> list) {
            assign(list.begin(), list.end());
        }

        template<typename Iter>
        void assign(Iter begin, Iter end) {
            clear();
            reserve(end - begin);

            base.data_last  = base.data_begin  + (end - begin);
            base.index_free = base.index_begin + (end - begin);
            base.erase_last = base.erase_begin + (end - begin);

            aul::uninitialized_copy(begin, end, base.data_begin, base.data_allocator);
            aul::uninitialized_iota(base.index_begin, base.index_free, base.data_begin, base.index_allocator);
            aul::uninitialized_iota(base.erase_begin, base.erase_last, 0, base.erase_allocator);
        }

        void assign(const size_type n, const T& val) {
            clear();
            reserve(n);

            base.data_last  = base.data_begin  + n;
            base.index_free = base.index_begin + n;
            base.erase_last = base.erase_begin + n;

            aul::uninitialized_fill(base.data_begin,  base.data_begin + n, val, base.data_allocator);
            aul::uninitialized_iota(base.index_begin, base.index_free, base.data_begin, base.index_allocator);
            aul::uninitialized_iota(base.erase_begin, base.erase_last, 0, base.erase_allocator);
        }

        void clear() noexcept {
            //Destroys data
            auto destroyer = [this] (index_type i) {
                if (i) {
                    std::allocator_traits<Alloc>::destroy(base.data_allocator, i.data());
                }
            };

            std::for_each(base.index_begin, base.index_end, destroyer);

            //Destroy index/erase values
            aul::destroy(base.index_begin, base.index_end, base.index_allocator);
            aul::destroy(base.erase_begin, base.erase_end, base.erase_allocator);

            //Reset capcity to 0
            base.clear();
        }

        void swap(Slot_map& right) noexcept(
            std::allocator_traits<Alloc>::propagate_on_container_swap::value ||
            std::allocator_traits<Alloc>::is_always_equal::value
        ) {
            std::swap(this->base, right.base);
        }

        void straighten();

        void sort();

        void stable_sort();

        //=====================================================================
        //	Assignment methods/operators
        //=====================================================================

        Slot_map& operator=(Slot_map& right) {
            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy constructible.");

            if (this == &right) {
                return *this;
            }

            clear();
            base = right.base;

            //Copy contruct data in this->base
            aul::uninitialized_copy(right.base. data_begin, right.base. data_end, base. data_begin, base. data_allocator);
            aul::uninitialized_copy(right.base.index_begin, right.base.index_end, base.index_begin, base.index_allocator);
            aul::uninitialized_copy(right.base.erase_begin, right.base.erase_end, base.erase_begin, base.erase_allocator);

            //Add offset to pointers in indicies to point to new data[]
            offset_indicies(right.base.data_begin, base.data_begin);

            //Update pointers
            base.data_last  = base.data_begin  + right.size();
            base.index_free = base.index_begin + (right.base.index_free - right.base.index_begin);
            base.erase_last = base.erase_begin + right.size();
        }

        Slot_map& operator=(Slot_map&& right) noexcept {
            if (this == &right) {
                return *this;
            }

            base = std::move(right.base);
            return *this;
        }

        Slot_map& operator=(const std::initializer_list<T> list) {
            clear();
            reserve(list.size());

            //Copy contruct data and default contruct index/erase values
            aul::uninitialized_copy(list.begin(), list.end(), base.data_begin, base.data_allocator);
            aul::uninitialized_iota(base.index_begin, base.index_end, base.data_begin, base.index_allocator);
            aul::uninitialized_iota(base.erase_begin, base.erase_end, 0, base.erase_allocator);

            base.data_last  = base.data_begin  + list.size();
            base.index_free = base.index_begin + list.size();
            base.erase_last = base.erase_begin + list.size();
        }

        //=====================================================================
        //	Access methods
        //=====================================================================

        reference front() {
            return *base.data_begin;
        }

        const_reference front() const {
            return *base.data_begin;
        }

        reference back() {
            return *(base.data_last);
        }

        const_reference back() const {
            return *(base.data_last);
        }

        key_type id_of(const_iterator it) {
            const_pointer p = it.operator->();

            return {
                base.erase_begin[p - base.data_begin],
                base.index_begin[p -base.data_begin].version()
            };
        }

        bool validate_id(key_type x) {
            return
                (x.version() == base.index_begin[x.data()].version()) &&
                (x.data() <= size());
        }

        reference at(const size_type x) {
            if (size() < x) {
                throw std::out_of_range("Index out of bounds.");
            }
            return base.data_begin[x];
        }

        const_reference at(const size_type x) const {
            if (size() < x) {
                throw std::out_of_range("Index out of bounds.");
            }
            return base.data_begin[x];
        }

        reference at(const key_type x) {
            if (size() < x.data()) {
                throw std::out_of_range("Index out of bounds.");
            }

            index_type index = base.index_begin[x.data()];

            if (x.version() != index.version()) {
                throw std::invalid_argument("Version mismatch.");
            }

            return *(index.data());
        }

        const_reference at(const key_type x) const {
            if (size() < x.data()) {
                throw std::out_of_range("Index out of bounds.");
            }

            index_type index = base.index_begin[x.data()];

            if (x.version() != index.version()) {
                throw std::invalid_argument("Version mismatch.");
            }

            return *(index.data());
        }

        //=====================================================================
        //	Access operators
        //=====================================================================

        reference operator[](const size_type x) {
            return base.data_begin[x];
        }

        const_reference operator[](const size_type x) const {
            return base.data_begin[x];
        }

        reference operator[](const key_type x) {
            index_type index = base.index_begin[x.data()];
            return *(index.data());
        }

        const_reference operator[](const key_type x) const {
            index_type index = base.index_begin[x.data()];
            return *(index.data());
        }

        //=====================================================================
        //	Modification methods
        //=====================================================================

        iterator insert(const_iterator it, const T& val) {
            emplace(it, val);
            return iterator(const_cast<pointer>(it.operator->()));
        }

        iterator insert(const_iterator it, T&& val) {
            emplace(it, std::move(val));
            return iterator(const_cast<pointer>(it.operator->()));
        }

        iterator stable_insert(const_iterator it, const T& val) {

        }

        iterator stable_insert(const_iterator it, T&& val) {

        }
        iterator insert(const_iterator it, const size_type n, const T& val);
            /*
            {
                size_type pos_index = it.pos - base.data_begin;

                grow(size() + n);
                //TODO: Implement

                //Number of existing elements to be moved right
                size_type move_count = std::max(0, base.data_last - it);

                for (int i = 0; i != n; ++i) {
                    move_construct_element(base.data_last - 1 - i, base.data_last - 1 - i + n);
                }

                return iterator(nullptr);
            }

            template<class InputIt>
            iterator insert(const_iterator it, InputIt begin, InputIt end) {
                //TODO: Implement

                return iterator(nullptr);
            }

            iterator insert(const_iterator it, std::initializer_list<T> list) {
                return insert(it, list.begin(), list.end());
            }
            */

        void erase(const key_type id) {
            if (!validate_id(id)) {
                throw std::invalid_argument("Version mismatch.");
            }

            size_type pos = base.index_begin[id.data()].data() - base.data_begin;
            //erase(begin() + pos);
        }

        /*
        */
        void erase(const_iterator x) {
            index_type* index = index_of(x.operator->());
            std::cout << index << std::endl;

            if (x.operator->() != (base.data_last - 1)) {
                swap_elements(const_cast<pointer>(x.operator->()), base.data_last - 1);
            }
            destruct_element(base.data_last - 1);

            find_free_index(index);
            --base.data_last;
            --base.erase_last;
        }

        void push_back(const T& val) {
            emplace_back(val);
        }

        void push_back(T&& val) {
            emplace_back(std::move(val));
        }

        void pop_back() {
            index_type* hint = index_of(base.data_last - 1);

            destruct_element(base.data_last - 1);

            --base.data_last;
            --base.erase_last;
            find_free_index(hint);
        }

        template<class... Args>
        reference emplace_back(Args&&... args) {
            grow(size() + 1);

            emplace_element(base.data_last, std::forward<Args>(args)...);

            find_free_index();
            ++base.data_last;
            ++base.erase_last;

            return base.data_last[-1];
        }

        template<class...Args>
        reference emplace(const_iterator pos, Args&&... args) {
            size_type position = pos.pos - base.data_begin;

            grow(size() + 1);

            pointer p = base.data_begin + position;

            //Conditionally move pointed to element to last;
            if (p != base.data_last) {
                move_construct_element(p, base.data_last);
            }

            //Construct element at p
            emplace_element(p, std::forward<Args>(args)...);
            std::cout << *p << std::endl;

            find_free_index();

            ++base.data_last;
            ++base.erase_last;

            return base.data_last[-1];
        }


        template<class...Args>
        reference stable_emplace(const_iterator pos, Args&&... args) {
            size_type position = pos.pos - base.data_begin;
        }


        //=====================================================================
        //	Iterator methods
        //=====================================================================

        iterator begin() noexcept {
            return iterator(base.data_begin);
        }

        const_iterator begin() const noexcept {
            return const_iterator(base.data_begin);
        }

        const_iterator cbegin() const noexcept {
            return const_iterator(base.data_begin);
        }



        iterator end() noexcept {
            return iterator(base.data_last);
        }

        const_iterator end()  const noexcept {
            return const_iterator(base.data_last);
        }

        const_iterator cend() const noexcept {
            return const_iterator(base.data_last);
        }



        reverse_iterator rbegin() noexcept {
            return reverse_iterator(base.data_last - 1);
        }

        const_reverse_iterator rbegin() const noexcept {
            return reverse_iterator(base.data_last - 1);
        }

        const_reverse_iterator crbegin() const noexcept {
            return const_reverse_iterator(base.data_last - 1);
        }



        reverse_iterator rend() noexcept {
            return reverse_iterator(base.data_begin);
        }

        const_reverse_iterator rend() const noexcept {
            return reverse_iterator(base.data_begin);
        }

        const_reverse_iterator crend() const noexcept {
            return const_reverse_iterator(base.data_begin);
        }

        //=====================================================================
        //	Capacity/size methods
        //=====================================================================

        [[nodiscard]] bool empty() const noexcept {
            return base.data_begin == base.data_end;
        }

        [[nodiscard]] size_type size() const noexcept {
            return base.data_last - base.data_begin;
        }

        [[nodiscard]] size_type max_size() const noexcept {
            constexpr size_type a = std::numeric_limits<difference_type>::max();
            size_type b = std::allocator_traits<Alloc>::max_size(base.data_allocator);
            return std::min(a, b);
        }

        inline void reserve(const size_type n) {
            //Return if new capacity is smaller or greater than current
            if (n <= capacity()) {
                return;
            }

            //Ensure Slot_map can grow
            if (max_size() < n) {
                throw std::length_error("Slot_map cannot grow.");
            }

            //Store element count and previous capcity
            size_type count = size();
            size_type old_capacity = capacity();

            //Pointer to newly allo0cated memory
            value_type*	data_begin  = nullptr;
            index_type* index_begin = nullptr;
            erase_type* erase_begin = nullptr;


            //Allocate new memory
            try {
                data_begin = base.data_allocator.allocate(n);
                index_begin = base.index_allocator.allocate(n);
                erase_begin = base.erase_allocator.allocate(n);
            } catch (...) {
                //Deallocate all memory if allocation fails
                base.data_allocator .deallocate(data_begin,  n);
                base.index_allocator.deallocate(index_begin, n);
                base.erase_allocator.deallocate(erase_begin, n);
                throw;
            }

            //Store the differences between the old and new locations in memory
            //difference_type data_offset  = data_begin - base.data_begin;
            //difference_type index_offset = index_begin - base.index_begin;
            //difference_type erase_offset = erase_begin - base.erase_begin;

            //Update index pointers
            offset_indicies(base.data_begin, data_begin);

            //Move data to new arrays.
            aul::uninitialized_move(base. data_begin, base. data_end, data_begin,  base. data_allocator);
            aul::uninitialized_move(base.index_begin, base.index_end, index_begin, base.index_allocator);
            aul::uninitialized_move(base.erase_begin, base.erase_end, erase_begin, base.erase_allocator);

            //create new indicies to fill empty space
            aul::uninitialized_fill(index_begin + old_capacity, index_begin + n, index_type(), base.index_allocator);

            base.data_begin = data_begin;
            base.data_last = base.data_begin + count;
            base.data_end = base.data_begin + n;

            base.index_free = index_begin + (base.index_free - base.index_begin);
            base.index_begin = index_begin;
            base.index_end = index_begin + n;

            base.erase_begin = erase_begin;
            base.erase_last = erase_begin + count;
            base.erase_end = erase_begin + n;
        }

        size_type capacity() const noexcept {
            return base.data_end - base.data_begin;
        }

        /*	Reducing capacity to be equal to size(). Potentially invalidates all
            IDs.
        */
        void shrink_to_fit() {
            //TODO: Ask allocator?
        }

        void resize(const size_type n, const_reference val) {
            //TODO:
        }

        void resize(const size_type n) {
            if (size() < n) {
                reserve(n);
            }

            //TODO
        }

        void set_growth_factor(const float fac) {
            if (fac <= 1.0f) {
                growth_factor = 1.1;
            } else {
                growth_factor = fac;
            }
        }

        //=====================================================================
        //	Operators
        //=====================================================================

        friend bool operator==(const Slot_map& lh, const Slot_map& rh) {
            if (lh.size() != rh.size()) {
                return false;
            }

            for (size_type i = 0; i < lh.size(); ++i) {
                if (lh[i] != rh[i]) {
                    return false;
                }
            }

            return true;
        }

        friend bool operator!=(const Slot_map& lh, const Slot_map & rh) {
            return !operator==(lh, rh);
        }

        friend bool operator<(const Slot_map& lh, const Slot_map& rh) {
            for (size_type i = 0; i < std::min(lh.size(), rh.size()); ++i) {
                if (lh[i] < rh[i]) {
                    return true;
                }
            }

            return lh.size() < rh.size();
        }

        friend bool operator>(const Slot_map& lh, const Slot_map& rh) {
            for (size_type i = 0; i < std::min(lh.size(), rh.size()); ++i) {
                if (lh[i] > rh[i]) {
                    return true;
                }
            }

            return lh.size() > rh.size();
        }

        friend bool operator<=(const Slot_map& lh, const Slot_map& rh) {
            return !operator>(lh, rh);
        }

        friend bool operator>=(const Slot_map& lh, const Slot_map& rh) {
            return !operator<(lh, rh);
        }

        //Internal access methods
        inline pointer data() noexcept {
            return base.data_begin;
        }

        inline const_pointer data() const noexcept {
            return base.data_begin;
        }

        inline index_type* indicies() noexcept {
            return base.index_begin;
        }

        inline const index_type* indicies() const noexcept {
            return base.index_begin;
        }

        inline erase_type* erase_table() noexcept {
            return base.erase_begin;
        }

        inline const erase_type* erase_table() const noexcept {
            return base.erase_begin;
        }

        //Allocator methods

        inline Alloc get_allocator() const noexcept {
            return base.data_allocator;
        }

        //Debugging methods

        void print() const {
            using std::cout;
            using std::endl;

            difference_type free_index_index = base.index_free - base.index_begin;
            difference_type data_last_index  = base.data_last  - base.data_begin;
            difference_type erase_last_index = base.erase_last - base.erase_begin;

            std::string partition = std::string(60, '=');

            cout << endl;
            cout << partition << endl;
            cout << "Slot_map using " << size() << '/' << capacity() << " slots." << endl;
            cout << "Last data  at " << data_last_index << '.' << endl;
            cout << "Free index at " << free_index_index << '.' << endl;
            cout << "Last erase at " << erase_last_index << '.' << endl;

            cout << endl;

            cout << "Data\t"  << base.data_begin  << '\t' << base.data_last  << '\t' << base.data_end  << endl;
            cout << "Index\t" << base.index_begin << '\t' << base.index_free << '\t' << base.index_end << endl;
            cout << "Erase\t" << base.erase_begin << '\t' << base.erase_last << '\t' << base.erase_end << endl;

            cout << endl;

            std::unique_ptr<bool[]> marked{new bool[capacity()]};

            for (unsigned i = 0; i < capacity(); ++i) {
                marked[i] = base.index_begin[i].data();
            }

            for(unsigned i = 0; i < capacity(); ++i) {
                cout << i << '\t';
                cout << std::setfill('0');
                cout << '(' << std::setw(9) << base.index_begin[i].data() << ", " << base.index_begin[i].version() << ")\t";

                if (marked[i]) {
                    cout << base.data_begin + i;
                } else {
                    cout << std::right << std::setw(9);
                    cout << (void*)nullptr;
                }

                cout << '\t';
                cout << base.erase_begin[i];

                cout << endl;
            }
            cout << partition << endl;
            cout << endl;
        }

        void print_elements() {
            using std::cout;
            using std::endl;

            cout << "Elements:" << endl;
            for(pointer p = base.data_begin; p != base.data_last; ++p) {
                cout << '[' << p << "] " << *p << endl;
            }
        }

    private:
        //=====================================================================
        //	Instance members
        //=====================================================================

        Slot_map_base base;
        float growth_factor = 2.0f;

        //=====================================================================
        //	Helper functions
        //=====================================================================

        inline index_type* index_of(const_pointer x) {
            return base.index_begin + (base.erase_begin[x - base.data_begin]);
        }

        /*	Offset all indicies in the current Slot_map to be realtive to the
            location of the new_begin pointer.
        */
        void offset_indicies(pointer old_begin, pointer new_begin) {
            auto offsetter = [old_begin, new_begin] (index_type& i) {
                if (i) {
                    i.data() = new_begin + (i.data() - old_begin);
                }
            };

            std::for_each(base.index_begin, base.index_end, offsetter);
        }

        /*	Increases the capacity of the current intance by multiplying the
            current capacity by the growth factor until it at least matches
            the target size.
        */
        void grow(const size_type target) {
            size_type new_capacity;

            if (capacity() == 0) {
                new_capacity = 1;
            } else {
                new_capacity = capacity();
            }

            while (new_capacity < target) {
                new_capacity = static_cast<size_type>(new_capacity * growth_factor);
            }

            reserve(new_capacity);
        }

        void destruct_element(pointer p) {
            std::allocator_traits<Alloc>::destroy(base.data_allocator, p);
            *index_of(p) = nullptr;
        }

        /*	Constructs an element at the specified position along with the
            corresponding erase value and index
        */
        template<class...Args>
        void emplace_element(pointer p, Args&&... args) {
            std::allocator_traits<Alloc>::construct(base.data_allocator, p, std::forward<Args>(args)...);

            base.index_free->data() = base.data_last;
            *base.erase_last = base.index_free - base.index_begin;
        }

        /*	Move constructs an element within the container from it's current
            position to dest, updates its index, and upates the erase value.
            Assumes that dest does not point to a position in data[] that is
            currently being used.
        */
        void move_construct_element(pointer from, pointer dest) {
            //Move construct at dest
            std::allocator_traits<Alloc>::construct(base.data_allocator, dest, std::move(*from));

            //Use free index to store new address
            base.index_free->data() = dest;

            //Assign erase index for newly created element
            base.erase_begin[base.index_free - base.index_begin] = dest - base.data_begin;
        }

        /*	Move assigns an element within the container from it's current
            position to dest, updates its index, and upates the erase value.
            Assumes that dest points to a position in data[] that is currently
            being used by an element.
        */
        void move_assign_element(pointer from, pointer dest) {
            erase_type* from_erase = base.erase_begin + (base.data_begin - from);
            erase_type* dest_erase = base.erase_begin + (base.data_begin - dest);

            index_type* from_index = base.index_begin + *from_erase;
            index_type* dest_index = base.index_begin + *dest_erase;

            dest_index->data() = from_index->data();
            *dest_erase = *from_erase;
            *dest = *from;
        }

        /*	Swaps the position of two elements along with their erase vaues
        */
        void swap_elements(pointer a, pointer b) {
            if (a == b) {
                return;
            }

            index_type& aIndex = *index_of(a);
            index_type& bIndex = *index_of(b);

            erase_type& aErase = base.erase_begin[a - base.data_begin];
            erase_type& bErase = base.erase_begin[b - base.data_begin];

            std::swap(aIndex.data(), bIndex.data());
            std::swap(aErase, bErase);
            std::swap(*a, *b);
        }

        /*	Moves index_free forward. The optional suggestion pointer should
            point to a location containing an empty index. If the suggestion is
            nullptr or it does not point to a valid location, moves index_free
            right until an empty index is found.
        */
        index_type& find_free_index(index_type* suggestion = nullptr) {
            //Check if suggestion is valid place for index_free;

            if (suggestion && !suggestion->data()) {
                base.index_free = suggestion;
            } else {
                //Otherwise search through indices until and empty is found or they run out
                while ((base.index_free->data() != nullptr) && (base.index_free < base.index_end)) {
                    ++base.index_free;
                }
            }

            return *base.index_free;
        }





        //=======================================
        //	Helper class
        //=======================================

        class Slot_map_base {
            using data_allocator_type  = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
            using index_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<index_type>;
            using erase_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<erase_type>;

            using data_allocator_traits  = std::allocator_traits<data_allocator_type>;
            using index_allocator_traits = std::allocator_traits<index_allocator_type>;
            using erase_allocator_traits = std::allocator_traits<erase_allocator_type>;

        public:
            index_type* index_begin = nullptr;
            index_type* index_free  = nullptr;
            index_type* index_end   = nullptr;

            value_type* data_begin = nullptr;
            value_type* data_last  = nullptr;
            value_type* data_end   = nullptr;

            erase_type* erase_begin = nullptr;
            erase_type* erase_last  = nullptr;
            erase_type* erase_end   = nullptr;

            data_allocator_type data_allocator;
            index_allocator_type index_allocator;
            erase_allocator_type erase_allocator;

            //Default constructor
            Slot_map_base(size_type n = 0, Alloc alloc = Alloc())
                :data_allocator{alloc},
                index_allocator{index_allocator_type(alloc)},
                erase_allocator{erase_allocator_type(alloc)}
            {
                allocate(n);
            }

            //Copy contructor
            Slot_map_base(const Slot_map_base& src)
                :data_allocator{ data_allocator_traits::select_on_container_copy_construction(src.data_allocator )},
                index_allocator{index_allocator_traits::select_on_container_copy_construction(src.index_allocator)},
                erase_allocator{erase_allocator_traits::select_on_container_copy_construction(src.erase_allocator)}
            {
                allocate(src.data_end - src.data_begin);
            }

            //Copy constructor with allocator
            Slot_map_base(const Slot_map_base& src, Alloc alloc)
                :data_allocator{ data_allocator_type(alloc)},
                index_allocator{index_allocator_type(alloc)},
                erase_allocator{erase_allocator_type(alloc)}
            {
                allocate(src.data_end - src.data_begin);
            }

            //Move constructor
            Slot_map_base(Slot_map_base&& src)
                :data_allocator{std::move(src.data_allocator )},
                index_allocator{std::move(src.index_allocator)},
                erase_allocator{std::move(src.erase_allocator)}
            {
                data_begin = src.data_begin;
                data_last = src.data_last;
                data_end = src.data_end;

                index_begin = src.index_begin;
                index_free = src.index_free;
                index_end = src.index_end;

                erase_begin = src.erase_begin;
                erase_last = src.erase_last;
                erase_end = src.erase_end;

                src.reset_pointers();
            }

            //Move constructor with allocator
            Slot_map_base(Slot_map_base&& src, Alloc alloc)
                :data_allocator{ data_allocator_type(alloc)},
                index_allocator{index_allocator_type(alloc)},
                erase_allocator{erase_allocator_type(alloc)}
            {
                data_begin = src.data_begin;
                data_last = src.data_last;
                data_end = src.data_end;

                index_begin = src.index_begin;
                index_free = src.index_free;
                index_end = src.index_end;

                erase_begin = src.erase_begin;
                erase_last = src.erase_last;
                erase_end = src.erase_end;

                src.reset_pointers();
            }

            //Destructor
            ~Slot_map_base() {
                deallocate(data_end - data_begin);
            }

            //Copy assignment operator
            Slot_map_base& operator=(const Slot_map_base& src) {
                deallocate(data_end - data_begin);

                 data_allocator = data_allocator_traits ::propagate_on_container_copy_assignment::value ? src.data_allocator  :  data_allocator_type();
                index_allocator = index_allocator_traits::propagate_on_container_copy_assignment::value ? src.index_allocator : index_allocator_type();
                erase_allocator = erase_allocator_traits::propagate_on_container_copy_assignment::value ? src.erase_allocator : erase_allocator_type();

                allocate(src.data_end - src.data_begin);

                return *this;
            }

            //Move assignment operator
            Slot_map_base& operator=(Slot_map_base&& src) noexcept {
                data_allocator  =  data_allocator_traits::propagate_on_container_move_assignment::value ? src.data_allocator  :  data_allocator_type();
                index_allocator = index_allocator_traits::propagate_on_container_move_assignment::value ? src.index_allocator : index_allocator_type();
                erase_allocator = erase_allocator_traits::propagate_on_container_move_assignment::value ? src.erase_allocator : erase_allocator_type();

                data_begin = src.data_begin;
                data_last = src.data_last;
                data_end = src.data_end;

                index_begin = src.index_begin;
                index_free = src.index_free;
                index_end = src.index_end;

                erase_begin = src.erase_begin;
                erase_last = src.erase_last;
                erase_end = src.erase_end;

                src.reset_pointers();

                return *this;
            }

            void clear() {
                deallocate(data_end - data_begin);
                reset_pointers();
            }

        private:

            void allocate(const size_type n) {
                if (n == 0) { return; }

                try {
                    data_begin  =  data_allocator.allocate(n);
                    index_begin = index_allocator.allocate(n);
                    erase_begin = erase_allocator.allocate(n);

                    data_last = data_begin;
                    index_free = index_begin;
                    erase_last = erase_begin;

                    data_end = data_begin + n;
                    index_end = index_begin + n;
                    erase_end = erase_begin + n;

                } catch (...) {
                    deallocate(n);
                    reset_pointers();

                    throw;
                }
            }

            void deallocate(const size_type n) {
                data_allocator .deallocate(data_begin,  n);
                index_allocator.deallocate(index_begin, n);
                erase_allocator.deallocate(erase_begin, n);
            }

            void reset_pointers() {
                data_begin  = data_last  = data_end  = nullptr;
                index_begin = index_free = index_end = nullptr;
                erase_begin = erase_last = erase_end = nullptr;
            }

        }; //End class Slot_map_base


    }; //End class aul::Slot_map<T, Alloc>





    template<class T>
    class Slot_map_ref {
    public:

        //-------------------------------------------------
        // -ctors
        //-------------------------------------------------

        Slot_map_ref() = default;
        ~Slot_map_ref() = default;

        //-------------------------------------------------
        // Conversion operators
        //-------------------------------------------------

        operator bool() const {
            return this->id.data() == -1;
        }

        typename aul::Slot_map<T>::reference operator*() {

        }

    private:

        aul::Versioned_type<
            typename aul::Slot_map<T>::difference_type,
            typename aul::Slot_map<T>::size_type
        > id = {-1, 0};

    };

}

#endif
>>>>>>> 47aeab6aa6728390e721b87c7337427e3419f775

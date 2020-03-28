#ifndef AUL_VECTOR_HPP
#define AUL_VECTOR_HPP

#include "Random_access_iterator.hpp"
#include "../memory/Memory.hpp"

#include <cstddef>
#include <iterator>
#include <memory>
#include <algorithm>
#include <iostream>
#include <type_traits>

namespace aul {

    /// \class Small_vector
    /// An SBO extended alternative to std::vector. An internal buffer is
    /// maintained on the stack with enough capacity to store at least SB_SIZE
    /// many elements. When the internal buffer is in use, the allocator goes
    /// unused and objects are constructed using placement new.
    ///
    /// \tparam T
    /// \tparam SB_SIZE
    /// \tparam Alloc
    template<typename T, int SB_SIZE = 0, class Alloc = std::allocator<T>>
    class Vector {

        class Small_vector_base;

    public:

        //-------------------------------------------------
        // Type aliases
        //-------------------------------------------------

        using value_type  = T;

        using reference = T&;
        using const_reference = const T&;

        using allocator_type = Alloc;

        using pointer = typename allocator_type::pointer;
        using const_pointer = typename allocator_type::const_pointer;

        using difference_type = typename allocator_type::difference_type;
        using size_type = typename allocator_type::size_type;

        using iterator = typename Random_access_iterator<aul::Allocator_types<allocator_type>, false>;
        using const_iterator = typename Random_access_iterator<aul::Allocator_types<allocator_type>, true>;
        using reverse_iterator = typename std::reverse_iterator<iterator>;
        using const_reverse_iterator = typename std::reverse_iterator<const_iterator>;

        //-------------------------------------------------
        // Constructors/Destructor
        //-------------------------------------------------

        /// \fun Vector()
        /// Default constructs a Vector
        ///
        Vector() noexcept(noexcept(allocator_type())) :
            base(0, allocator_type())
        {}

        /// \fun Vector(const allocator_type&)
        /// Constructs a Vector by copy constructing
        /// \param allocator Sour
        explicit Vector(const allocator_type& allocator) noexcept:
            base(0, allocator())
        {}

        /// \fun Vector(const size_type, const T, const allocator_type)
        ///
        /// \param n
        /// \param val
        /// \param allocator
        Vector(const size_type n, const T val, const allocator_type& allocator = {}):
            base(0, allocator)
        {
            if (n <= SB_SIZE) {
                std::uninitialized_fill_n(n, val);
            } else {
                aul::uninitialized_fill_n(n, val, base.allocator);
            }
        }

        Vector(const std::initialize_list<T> list, const allocator_type& allocator);

        Vector(const std::initialize_list<T> list, const allocator_type& allocator = {});



        template<class Iter>
        Vector(Iter begin, Iter end, const allocator_type& allocator);

        template<class Iter>
        Vector(Iter begin, Iter end, const allocator_type& allocator = {});



        Vector(const Vector& vec);

        Vector(const Vector& vec, const allocator_type& allocator);



        Vector(Vector&& vec);

        Vector(Vector&& vec, const allocator_type& allocator);



        ~Vector() {
            clear();
        }

        //-------------------------------------------------
        // Assignment operators
        //-------------------------------------------------

        Vector& operator=(const Vector& vec) {
            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy contructible");
            clear();

            //TODO: Implement
        }

        Vector& operator=(Vector&& vec) noexcept {
            //Tests for default constructed allocator's ability to manage memory
            if (
                std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value ||
                base.allocator == vec.base.allocator
                ) {
                base = Vector_base(vec.base);
            } else {
                //Otherwise allocate new memory and move data there

                base = Vector_base(vec.end - vec.begin);
                aul::uninitialized_move(vec.base.begin, vec.base.end, base.begin, base.allocator);
            }
        }

        Vector& operator=(const std::initializer_list<T>& r) {
            static_assert(std::is_copy_constructible<T>::value, "Type T is not copy contructible");
            assign(r);
        }

        void assign(const size_type n, const T& val) {
            reserve(n);
            aul::uninitialized_fill(base.begin, base.end, val, base.allocator);
        }

        template<class Iter>
        void assign(Iter begin, Iter end) {
            reserve(end - begin);
            aul::uninitialized_copy(begin, end, base.begin, base.allocator_type);
        }

        void assign(const std::initializer_list<T>& list) {
            reserve(list.end() - list.begin());
            aul::uninitialized_copy(list.begin(), list.end(), base.begin, base.allocator);
        }

        inline const allocator_type& get_allocator_typeator() const {
            return base.allocator;
        }

        //---------------------------------------------------------------------
        // Access methods
        //---------------------------------------------------------------------

        reference at(size_type pos) {
            if (size() <= pos) {
                throw std::out_of_range("Error attempt to access out of range");
            }

            return *(base.begin + pos);
        }

        const_reference at(size_type pos) const {
            if (size() <= pos) {
                throw std::out_of_range("Error attempt to access out of range");
            }

            return *(base.begin + pos);
        }

        reference operator[](size_type pos) {
            return *(base.begin + pos);
        }

        const_reference operator[](size_type pos) const {
            return *(base.begin + pos);
        }

        reference front() {
            return *(base.begin);
        }

        const_reference front() const {
            return *(base.begin);
        }

        reference back() {
            return *(base.last - 1);
        }

        const_reference back() const {
            return *(base.last - 1);
        }

        pointer data() {
            return base.begin;
        }

        const_pointer data() const {
            return base.begin;
        }

        //---------------------------------------------------------------------
        // Capacity methods
        //---------------------------------------------------------------------

        [[nodiscard]] inline bool empty() const noexcept {
            return base.last - base.begin;
        }

        inline size_type size() const noexcept {
            return base.last - base.begin;
        }

        size_type max_size() const noexcept {
            size_type x = std::allocator_traits<allocator_type>::max_size(base.allocator);
            size_type y = std::numeric_limits<size_type>::max();

            return std::min(x, y);
        }

        constexpr size_type small_buffer_size() const noexcept {
            return SB_SIZE:
        }

        void reserve(size_type n) {
            if (n > max_size()) {
                throw std::length_error("Vector has reached max_size()");
            }

            Vector_base new_base(n);
            aul::uninitialized_move(base.begin, base.end, new_base.begin, base.allocator);
            base = std::move(new_base);
        }

        inline size_type capacity() const noexcept {
            return base.end - base.begin;
        }

        void shrink_to_fit();

        //---------------------------------------------------------------------
        // Mutator methods
        //---------------------------------------------------------------------

        template<class...Args>
        iterator emplace(const_iterator pos, Args...args) {
            if ((pos.operator->() < base.begin) || (base.last <= pos.operator->())) {
                throw std::out_of_range("Attempting to contruct element at position indicated by invalid iterator.");
            }

            pointer p = const_cast<pointer>(pos.operator->());
            base.allocator_type.construct(base.end, std::move(*(base.end - 1)));
            std::move_backward(p, base.end - 1, base.end);

            ++base.end;
            ++base.last;

            return iterator(p);
        }

        template<class...Args>
        iterator emplace_back(const_iterator pos, Args... args) {
            return emplace(end(), std::forward<Args>(args)...);
        }

        iterator insert(const_iterator pos, const T& val) {
            return emplace(pos, std::move(val));
        }

        iterator insert(const_iterator pos, T&& value) {
            emplace(pos, std::move(value));
        }

        template<class Iter>
        iterator insert(const_iterator pos, Iter begin, Iter end) {
            if ((pos.operator->() < base.begin) || (base.last <= pos.operator->())) {
                throw std::out_of_range("Attempting to contruct element at position indicated by invalid iterator.");
            }

            pointer p = const_cast<pointer>(pos.operator->());

            grow(size() + (end - begin));

            //TODO: Insert and offset elements

            return iterator(p);
        }

        iterator insert(const_iterator pos, std::initializer_list<T>& list) {
            return insert(pos, list.begin(), list.end());
        }

        void push_back(const T& value) {
            //TODO
        }

        void push_back(T&& value) {
            //TODO
        }

        inline void clear() noexcept {
            aul::destroy(base.begin, base.last, base.allocator);
            if (!is_using_sbo()) {

            }
        }

        void swap(Vector& vec) noexcept {
            std::swap(base, vec.base);
        }

        //---------------------------------------------------------------------
        // Comparison operators
        //---------------------------------------------------------------------

        friend bool operator==(const Vector& lhs, const Vector& rhs) noexcept {
            if (lhs.size() == rhs.size()) {
                return true;
            }

            for (size_type i = 0; i < lhs.size(); ++i) {
                if (lhs[i] != rhs[i]) {
                    return false;
                }
            }

            return true;
        }

        friend bool operator!=(const Vector& lhs, const Vector& rhs) noexcept {
            return !operator==(lhs, rhs);
        }



        //---------------------------------------------------------------------
        // Iterator methods
        //---------------------------------------------------------------------

        inline iterator begin() noexcept {
            return iterator(base.begin);
        }

        inline const_iterator begin() const noexcept {
            return const_iterator(base.begin);
        }

        inline const_iterator cbegin() const noexcept {
            return const_iterator(base.begin);
        }


        inline iterator end() noexcept {
            return iterator(base.last);
        }

        inline const_iterator end() const noexcept {
            return const_iterator(base.last);
        }

        inline const_iterator cend() const noexcept {
            return const_iterator(base.last);
        }


        inline reverse_iterator rbegin() noexcept {
            return reverse_iterator(base.last - 1);
        }

        inline const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(base.last - 1);
        }

        inline const_reverse_iterator crbegin() const noexcept {
            return const_reverse_iterator(base.last - 1);
        }


        inline reverse_iterator rend() noexcept {
            return reverse_iterator(base.begin - 1);
        }

        inline const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(base.begin - 1);
        }

        inline const_reverse_iterator crend() const noexcept {
            return const_reverse_iterator(base.begin - 1);
        }

    private:

        //---------------------------------------------------------------------
        // Instance variables
        //---------------------------------------------------------------------

        Vector_base base;

        inline bool is_using_sbo() const noexcept {
            return (size() <= SB_SIZE);
        }

        void grow(size_type n);

        //=====================================================================
        // Helper classes
        //=====================================================================

        class Vector_base {
        public:

            //-----------------------------------------------------------------
            // Instance variables
            //-----------------------------------------------------------------

            /// \var sbo
            /// Internal buffer
            ///
            T sbo[SBO_SIZE];

            /// \var begin
            /// Points to the first element within the currently used array.
            /// When (end - begin) <= SBO_SIZE, begin == sbo.
            ///
            pointer begin = sbo;

            /// /var last
            /// Points to one past the last constructed elements in the array.
            ///
            pointer last = sbo;

            /// \var end
            /// Points to one past the end of the allocated array. When (end -
            /// begin) <= SBO_SIZE, end == sbo + SBO_SIZE;
            ///
            pointer end = sbo + SBO_SIZE;

            allocator_type allocator{};

            //-----------------------------------------------------------------
            // Constructors/Destructor
            //-----------------------------------------------------------------

            Vector_base(const size_type n = 0u, const allocator_type& allocator):
                allocator(allocator)
            {
                allocate(n);
            }

            Vector_base(const Vector_base& r);

            Vector_base(Vector_base&& r);

            //-----------------------------------------------------------------
            // Operators
            //-----------------------------------------------------------------

            Vector_base& operator=(const Vector_base& base);

            Vector_base& operator=(Vector_base&& vec) noexcept;

        private:

            /// \fn allocate
            ///
            /// Allocates at least enough memory to store n elements. Grabs
            /// the entire internal buffer if n is smaller than SBO_SIZE. Grabs
            /// memory from the allocator otherwise.
            ///
            /// \param n Number of elements to allocate memory for
            void allocate(const size_type n) {
                if (n < SBO_SIZE) {
                    return;
                }

                try {
                    if (begin) {
                        begin = allocator.allocate(n, begin);
                    } else {
                        begin = allocator.allocate(n);
                    }
                    last = begin;
                    end = begin + n;

                } catch (...) {
                    allocator.deallocate(begin, n);
                    begin = nullptr;
                    last = nullptr;
                    end = nullptr;

                    throw;
                }
            }

            /// \fn deallocate
            ///
            /// Releases memory allocated from allocator and activates internal
            /// buffer. Does nothing if internal buffer is already active.
            void deallocate() {
                if ((end - begin) <= SBO_SIZE) {
                    return;
                }

                try {
                    std::allocator_traits<allocator_type>::deallocate(allocator, begin, (end - begin));
                    begin = sbo;
                    last = sbo;
                    end = sbo + SBO_SIZE;

                } catch (...) {
                    throw;
                }
            }

        };    //End class Vector_base<T>

    }; //End class template Vector<T>

}

#endif

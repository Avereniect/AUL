//
// Created by avereniect on 7/19/20.
//

#ifndef AUL_VIRTUAL_ALLOCATOR_HPP
#define AUL_VIRTUAL_ALLOCATOR_HPP

#include "../Math.hpp"

#include <type_traits>
#include <memory>

namespace aul {

    ///
    /// \tparam T Type pointer points to
    /// \tparam I Backing signed integral type
    /// \tparam S Stride. Must be less than or equal to sizeof(T)
    /// \tparam is_const
    template<class T, class I, I Stride = 1, bool is_const = false>
    class Relative_pointer {

        static_assert(std::is_signed_v<I>);
        static_assert(aul::is_pow2(Stride));
        static_assert(Stride <= sizeof(T), "Stride is too large to address consecutive elements of type T");

    public:
        //=================================================
        // -ctors
        //=================================================

        Relative_pointer() = default;

        ///
        /// Construct pointer null pointer
        ///
        Relative_pointer(const std::nullptr_t):
            offset(0) {}

        ///
        /// \param ptr Primitve pointer to point to. Value is assumed to be an
        /// address aligned to Stride
        ///
        Relative_pointer(const T* ptr):
            offset((reinterpret_cast<char*>(this) - reinterpret_cast<const char*>(ptr)) * sizeof(T) / Stride) {}

        ///
        /// \param ptr
        ///
        Relative_pointer(Relative_pointer&& ptr) noexcept :
            offset() {}

        Relative_pointer(const Relative_pointer& ptr) :
            offset(*this) {}

        ~Relative_pointer() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Relative_pointer& operator=(const Relative_pointer& rhs) {
            char* ptr0 = std::addressof(*this);
            char* ptr1 = std::addressof(rhs);

            offset += sizeof(T) * (ptr1 - ptr0) / Stride;

            return *this;
        }

        Relative_pointer& operator=(Relative_pointer&& rhs) noexcept {
            char* ptr0 = std::addressof(*this);
            char* ptr1 = std::addressof(rhs);

            offset += sizeof(T) * (ptr1 - ptr0) / Stride;

            return *this;
        }

        Relative_pointer& operator=(const std::nullptr_t) {
            offset = 0;
            return *this;
        }

        //=================================================
        // Arithmetic assignment operators
        //=================================================

        Relative_pointer& operator+=(const I o) {
            offset += o;
            return *this;
        }

        Relative_pointer& operator-=(const I o) {
            offset -= o;
            return *this;
        }

        //=================================================
        // Arithmetic operators
        //=================================================

        Relative_pointer operator+(const I o) const {
            offset += o;
            return *this;
        }

        friend Relative_pointer operator+(const I o, const Relative_pointer& ptr) {
            return ptr + o;
        }

        Relative_pointer operator-(const I o) const {
            offset -= o;
            return *this;
        }

        I operator-(const Relative_pointer& rhs) {
            return offset - rhs.offset;
        }

        friend I operator-(const std::nullptr_t, const Relative_pointer& rhs) {
            return (0 - rhs.offset);
        }

        //=================================================
        // Comparison operators
        //=================================================

        bool operator==(const Relative_pointer& ptr) {
            return (this->operator->() == ptr.operator->());
        }

        bool operator==(const std::nullptr_t) {
            return (offset == 0);
        }

        bool operator!=(const Relative_pointer& ptr) {
            return (this->operator->() != ptr.operator->());
        }

        bool operator!=(const std::nullptr_t) {
            return (offset != 0);
        }

        bool operator>=(const Relative_pointer& ptr) const {
            return (this->operator->() >= ptr.operator->());
        }

        bool operator>=(const std::nullptr_t ptr) const {
            return (offset >= 0);
        }

        bool operator<=(const Relative_pointer& ptr) const {
            return (this->operator->() <= ptr.operator->());
        }

        bool operator<=(const std::nullptr_t ptr) const {
            return (offset <= 0);
        }

        bool operator>(const Relative_pointer& ptr) const {
            return (this->operator->() > ptr.operator->());
        }

        bool operator>(const std::nullptr_t ptr) const {
            return (offset > 0);
        }

        bool operator<(const Relative_pointer& ptr) const {
            return (this->operator->() < ptr.operator->());
        }

        bool operator<(const std::nullptr_t ptr) const {
            return (offset < 0);
        }

        //=================================================
        // Increment/decrement operators
        //=================================================

        Relative_pointer& operator++() {
            offset += sizeof(T) / Stride;
            return *this;
        }

        Relative_pointer operator++(int) {
            Relative_pointer temp = *this;
            offset += sizeof(T) / Stride;
            return temp;
        }

        Relative_pointer& operator--() {
            offset -= sizeof(T) / Stride;
            return *this;
        }

        Relative_pointer operator--(int) {
            Relative_pointer temp = *this;
            offset -= sizeof(T) / Stride;
            return temp;
        }

        //=================================================
        // Dereference operators
        //=================================================

        std::conditional<is_const, const T&, T&> operator*() const {
            return *(this->operator->());
        }

        std::conditional<is_const, const T*, T*> operator->() const {
            char* ptr[sizeof(T)] = std::addressof(*this);
            ptr += offset * Stride;
            return reinterpret_cast<T*>(ptr);
        }

        //=================================================
        // Conversion operator
        //=================================================

        operator bool() const {
            return (offset != 0);
        }

        explicit operator std::conditional<is_const, const T*, T*>() const {
            return operator->();
        }

    private:

        //=================================================
        // Instance members
        //=================================================

        I offset;

    };

}



namespace std {

    template<class T, class I, I Stride, bool is_const>
    class pointer_traits<aul::Relative_pointer<T, I, Stride, is_const>> {
    public:

        using pointer = aul::Relative_pointer<T, I, Stride, is_const>;
        using elemment_type = T;
        using difference_type = std::make_unsigned<I>;

        template<class U>
        using rebind = aul::Relative_pointer<U, I, Stride, is_const>;

        //TODO: C++20 Make constexpr
        static pointer pointer_to(const T& t) noexcept {
            return pointer{t};
        }

    };

    //TODO: C++ 20 specialize std::to_address

}

namespace {

    auto alignment = [] (const std::size_t a, const std::size_t b) {
        return std::max(a, b);
    };

    template<class T, class Uint, Uint Stride>
    struct alignas(alignment(Stride, alignof(Uint))) Pool_header {
        Uint user_count;
        Uint capacity;
    };

    template<class T, class Uint, Uint Stride>
    struct alignas(Stride) Block_header {
        Uint capacity;
    };

}

namespace aul {

    ///
    /// \tparam T Type to point to
    /// \tparam I Unsigned integral type to use for offset pointers
    /// \tparam Stride Size of gaps between addresses pointer can handle
    template<class T, class I, I Stride = 1>
    class Virtual_allocator {

        using pool_header = Pool_header<T, I, Stride>;
        using block_header = Block_header<T, I, Stride>;

    public:

        static_assert(std::is_integral_v<I>);
        static_assert(std::is_signed_v<I>);
        static_assert(aul::is_pow2(Stride));
        static_assert(Stride <= sizeof(T));

        //=================================================
        // Type aliases
        //=================================================

        using pointer = Relative_pointer<T, I, Stride, false>;
        using const_pointer = Relative_pointer<T, I, Stride, true>;

        using void_pointer = Relative_pointer<void, I, Stride, false>;
        using const_void_pointer = Relative_pointer<void, I, Stride, true>;

        using value_type = T;

        using size_type = I;
        using difference_type = std::make_signed<I>;

        template<class U>
        class rebind {
            using other = Virtual_allocator<U, I, Stride>;
        };

        static constexpr bool is_always_equal = false;
        static constexpr bool propagate_on_container_copy_assignment = true;
        static constexpr bool propagate_on_container_move_assignment = true;
        static constexpr bool propagate_on_container_swap = true;

        //=================================================
        // -ctors
        //=================================================

        Virtual_allocator() = default;

        ///
        /// \param element_count Number of elements to allocate memory for
        ///
        Virtual_allocator(const size_type element_count) {
            auto byte_count = element_count * Stride + sizeof(pool_header) + sizeof(block_header);
            pool = reinterpret_cast<std::byte*>(aligned_alloc(Stride, byte_count));


            pool_header& pool_header = *(new(pool) Virtual_allocator::pool_header);
            pool_header.user_count = 1;
            pool_header.capacity = element_count;

            pool += sizeof(pool_header);
            block_header& first_block_header = *(new(pool) Virtual_allocator::block_header);
            first_block_header.capacity = element_count - sizeof(pool_header);

            pool += element_count * sizeof(T) - sizeof(pool_header);
            block_header& last_block_header = *(new(pool) Virtual_allocator::block_header);
            last_block_header.capacity = 0;
        }

        ~Virtual_allocator() {
            if (pool) {
                --pool_users();
                if (pool_users()) {
                    free(pool);
                }
            }
        }

        //=================================================
        // Assignment operators
        //=================================================

        Virtual_allocator& operator=(const Virtual_allocator&);
        Virtual_allocator& operator=(Virtual_allocator&&);

        //=================================================
        // Allocation methods
        //=================================================

        pointer allocate(const size_type n);
        pointer allocate(const size_type n, const_pointer hint);

        pointer deallocate(pointer);

        //=================================================
        // Comparison operators
        //=================================================

        bool operator==(const Virtual_allocator& allocator) {
            return (pool == allocator.pool);
        }

        bool operator!=(const Virtual_allocator& allocator) {
            return (pool != allocator.pool);
        }

        //=================================================
        // Misc. methods
        //=================================================

        size_type max_size() const {
            constexpr auto diff_max = std::numeric_limits<difference_type>::max();
            return std::min(static_cast<size_type>(diff_max), capacity());
        }

    private:
        std::byte* pool = nullptr;

        //=================================================
        // Helper functions
        //=================================================

        size_type& capacity() {
            auto& pool_header = *reinterpret_cast<Virtual_allocator::pool_header*>(pool);
            return pool_header.capacity;
        }

        size_type& pool_users() {
            auto& pool_header = *reinterpret_cast<Virtual_allocator::pool_header*>(pool);
            return pool_header.user_count;
        }

    };

}

#endif //AUL_VIRTUAL_ALLOCATOR_HPP

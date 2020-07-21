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
    template<class T, class I, I S = 1, bool is_const = false>
    class Relative_pointer {

        static_assert(std::is_signed_v<I>);
        static_assert(aul::is_pow2(S));
        static_assert(S <= sizeof(T), "Stride is too large to address consecutive elements of type T");

    public:
        //=================================================
        // -ctors
        //=================================================

        Relative_pointer() = default;

        Relative_pointer(const std::nullptr_t):
            offset(0) {}

        Relative_pointer(const T* ptr):
            offset((this - ptr) * sizeof(T) / S) {}

        Relative_pointer(const I offset):
            offset(offset) {}

        Relative_pointer(Relative_pointer&& ptr) :
            offset() {}

        Relative_pointer(const Relative_pointer& ptr) :
            offset(*this) {}

        ~Relative_pointer();

        //=================================================
        // Assignment operators
        //=================================================

        Relative_pointer& operator=(const Relative_pointer& rhs) {
            auto* ptr0 = std::addressof(*this);
            auto* ptr1 = std::addressof(rhs);

            offset += (ptr1 - ptr0);

            return *this;
        }

        Relative_pointer& operator=(Relative_pointer&& rhs) {
            auto* ptr0 = std::addressof(*this);
            auto* ptr1 = std::addressof(rhs);

            offset += sizeof(T) / S * (ptr1 - ptr0);

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
            offset += sizeof(T) / S;
            return *this;
        }

        Relative_pointer operator++(int) {
            Relative_pointer temp = *this;
            offset += sizeof(T) / S;
            return temp;
        }

        Relative_pointer& operator--() {
            offset -= sizeof(T) / S;
            return *this;
        }

        Relative_pointer operator--(int) {
            Relative_pointer temp = *this;
            offset -= sizeof(T) / S;
            return temp;
        }

        //=================================================
        // Dereference operators
        //=================================================

        T& operator*() {
            return *(this->operator->());
        }

        const T& operator*() const {
            return *(this->operator->());
        }

        T* operator->() {
            char* ptr[sizeof(T)] = std::addressof(*this);
            ptr += offset;
            return reinterpret_cast<T*>(ptr);
        }

        const T* operator->() const {
            char* ptr[sizeof(T)] = std::addressof(*this);
            ptr += offset;
            return reinterpret_cast<T*>(ptr);
        }

    private:

        //=================================================
        // Instance members
        //=================================================

        I offset;

    };

}

namespace std {

    template<class T, class I, I S, bool is_const>
    class pointer_traits<aul::Relative_pointer<T, I, S, is_const>> {
    public:

        using pointer = aul::Relative_pointer<T, I, S, is_const>;
        using elemment_type = T;
        using difference_type = std::make_unsigned<I>;

        template<class U>
        using rebind = aul::Relative_pointer<U, I, S, is_const>;

        //TODO: C++20 Make constexpr
        static pointer pointer_to(const T& t) noexcept {
            return pointer{t};
        }

    };

    //TODO: C++ 20 specialize std::to_address

}

namespace aul {

    ///
    /// \tparam T Type to point to
    /// \tparam I
    /// \tparam S
    template<class T, class I, I S>
    class Virtual_allocator {
    public:

        static_assert(std::is_integral_v<T>);
        static_assert(std::is_signed_v<T>);
        static_assert(aul::is_pow2(S));
        static_assert(S <= sizeof(T));

        //=================================================
        // Type aliases
        //=================================================

        using pointer = Relative_pointer<T, I, S, false>;
        using const_pointer = Relative_pointer<T, I, S, true>;

        using void_pointer = Relative_pointer<void, I, S, false>;
        using const_void_pointer = Relative_pointer<void, I, S, true>;

        using value_type = T;

        using size_type = std::make_unsigned<I>;
        using difference_type = I;

        template<class U>
        class rebind {
            using other = Virtual_allocator<U, I, S>;
        };

        //=================================================
        // -ctors
        //=================================================

        Virtual_allocator() = default;

        ~Virtual_allocator() {

        }

        //=================================================
        // Assignment operators
        //=================================================

        Virtual_allocator& operator=(const Virtual_allocator&);
        Virtual_allocator& operator=(Virtual_allocator&&);

        pointer allocate(const size_type n);
        pointer deallocate(pointer);

    private:
        std::size_t capacity = 0;

    };

}

#endif //AUL_VIRTUAL_ALLOCATOR_HPP

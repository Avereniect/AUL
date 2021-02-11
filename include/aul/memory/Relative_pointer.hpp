//
// Created by avereniect on 7/19/20.
//

#ifndef AUL_RELATIVE_POINTER_HPP
#define AUL_RELATIVE_POINTER_HPP

#include "../Bits.hpp"

#include <type_traits>
#include <memory>

namespace aul {

    ///
    /// \tparam T Type pointer points to
    /// \tparam I Backing signed integral type
    /// \tparam S Stride. Must be less than or equal to sizeof(T)
    /// \tparam is_const
    template<class T, class Int, I Stride = 1, bool is_const = false>
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
        explicit Relative_pointer(const T* ptr):
            offset((reinterpret_cast<char*>(this) - reinterpret_cast<const char*>(ptr)) * sizeof(T) / Stride) {}

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

        Relative_pointer& operator+=(const Int o) {
            offset += o;
            return *this;
        }

        Relative_pointer& operator-=(const Int o) {
            offset -= o;
            return *this;
        }

        //=================================================
        // Arithmetic operators
        //=================================================

        Relative_pointer operator+(const Int o) const {
            offset += o;
            return *this;
        }

        friend Relative_pointer operator+(const Int o, const Relative_pointer& ptr) {
            return ptr + o;
        }

        Relative_pointer operator-(const Int o) const {
            offset -= o;
            return *this;
        }

        Int operator-(const Relative_pointer& rhs) {
            return offset - rhs.offset;
        }

        friend Int operator-(const std::nullptr_t, const Relative_pointer& rhs) {
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

        std::conditional<is_const, const T&, T&> operator[](const Int x) const {
            return *(*this + x);
        }

        //=================================================
        // Conversion operators
        //=================================================

        ///
        /// Conversion to bool
        ///
        /// \return True if value is not equivalent to nullptr
        ///
        operator bool() const {
            return (offset != 0);
        }

        ///
        /// Conversion to raw pointer
        ///
        /// \return Pointer 
        ///
        explicit operator std::conditional<is_const, const T*, T*>() const {
            return operator->();
        }

    private:

        //=================================================
        // Instance members
        //=================================================

        ///
        /// Offset from address of this object
        ///
        Int offset;

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

}

#endif //AUL_RELATIVE_POINTER_HPP

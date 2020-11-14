//
// Created by avereniect on 7/19/20.
//

#ifndef AUL_RELATIVE_POINTER_HPP
#define AUL_RELATIVE_POINTER_HPP

#include "../Math.hpp"

#include <memory>

namespace aul {

    ///
    /// \tparam T Type pointer points to
    /// \tparam Int Backing signed integral type
    /// \tparam stride Distance between consecutive addresses which may be represented.
    /// \tparam is_const
    template<class T, class Int, Int stride = 1, bool is_const = false>
    class Relative_pointer_impl {
    private:

        static_assert(std::is_signed_v<Int>);
        static_assert(aul::is_pow2(stride));
        static_assert(stride <= sizeof(T), "Stride is too large to address consecutive elements of type T");

        //=================================================
        // Static details
        //=================================================

        ///
        /// Offset value to be used to represent a null pointer. If the
        /// pointer's stride is larger than Relative_pointer then an offset of
        /// 1 is never a valid address as it points to the middle of the
        /// current object. Otherwise, an offset of 0 is ued. Note that an
        /// offset of 0 may not allow node structs to point to themselves if the
        /// relative pointer which is meant to point to the node is the node's
        /// first member.
        ///
        static constexpr Int null_offset = (stride > sizeof(Int)) ? 1 : 0;

        ///
        /// Value to add to offset when incrementing/decrementing the pointer.
        ///
        static constexpr auto delta = sizeof(T) / stride;

        using absolute_pointer = std::conditional_t<is_const, const T*, T*>;

    public:

        //=================================================
        // -ctors
        //=================================================

        Relative_pointer_impl() = default;

    private:

        explicit Relative_pointer_impl(const Int offset):
            offset(offset) {}

    public:

        ///
        /// Construct pointer null pointer
        ///
        Relative_pointer_impl(const std::nullptr_t):
            offset(null_offset) {}

        ///
        /// \param ptr Primitve pointer to point to. Value is assumed to be an
        /// address aligned to Stride
        ///
        explicit Relative_pointer_impl(const T* ptr):
            offset((reinterpret_cast<const char*>(ptr) - reinterpret_cast<const char*>(this)) / stride) {}

        Relative_pointer_impl(Relative_pointer_impl&& ptr) noexcept :
            offset(ptr.offset + (reinterpret_cast<char*>(std::addressof(ptr)) - reinterpret_cast<const char*>(this)) / stride) {}

        Relative_pointer_impl(const Relative_pointer_impl& ptr) :
            offset(ptr.offset + (reinterpret_cast<const char*>(std::addressof(ptr)) - reinterpret_cast<const char*>(this)) / stride) {}

        ~Relative_pointer_impl() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Relative_pointer_impl& operator=(const Relative_pointer_impl& rhs) {
            auto* ptr0 = reinterpret_cast<char*>(this);
            auto* ptr1 = reinterpret_cast<const char*>(std::addressof(rhs));

            offset = rhs.offset + (ptr1 - ptr0) / stride;

            return *this;
        }

        Relative_pointer_impl& operator=(Relative_pointer_impl&& rhs) noexcept {
            auto* ptr0 = reinterpret_cast<char*>(this);
            auto* ptr1 = reinterpret_cast<char*>(std::addressof(rhs));

            offset = rhs.offset + (ptr1 - ptr0) / stride;

            return *this;
        }

        Relative_pointer_impl& operator=(absolute_pointer ptr) {
            auto* ptr0 = reinterpret_cast<const char*>(this);
            auto* ptr1 = reinterpret_cast<const char*>(ptr);

            offset += (ptr1 - ptr0) * sizeof(T) / stride;

            return *this;
        }

        Relative_pointer_impl& operator=(const std::nullptr_t) {
            offset = null_offset;
            return *this;
        }

        //=================================================
        // Arithmetic assignment operators
        //=================================================

        Relative_pointer_impl& operator+=(const Int o) {
            offset += o;
            return *this;
        }

        Relative_pointer_impl& operator-=(const Int o) {
            offset -= o;
            return *this;
        }

        //=================================================
        // Arithmetic operators
        //=================================================

        Relative_pointer_impl operator+(const Int o) const {
            auto ret = *this;
            ret.offset += o;
            return *this;
        }

        friend Relative_pointer_impl operator+(const Int o, const Relative_pointer_impl& ptr) {
            return ptr + o;
        }

        Relative_pointer_impl operator-(const Int o) const {
            offset -= o;
            return *this;
        }

        Int operator-(const Relative_pointer_impl& rhs) {
            return offset - rhs.offset;
        }

        friend Int operator-(const std::nullptr_t, const Relative_pointer_impl& rhs) {
            return (null_offset - rhs.offset);
        }

        //=================================================
        // Comparison operators
        //=================================================

        bool operator==(const Relative_pointer_impl& ptr) {
            return (this->operator->() == ptr.operator->());
        }

        bool operator==(const std::nullptr_t) {
            return (offset == null_offset);
        }

        bool operator!=(const Relative_pointer_impl& ptr) {
            return (this->operator->() != ptr.operator->());
        }

        bool operator!=(const std::nullptr_t) {
            return (offset != null_offset);
        }

        bool operator>=(const Relative_pointer_impl& ptr) const {
            return (this->operator->() >= ptr.operator->());
        }

        bool operator>=(const std::nullptr_t) const {
            return (offset >= null_offset);
        }

        bool operator<=(const Relative_pointer_impl& ptr) const {
            return (this->operator->() <= ptr.operator->());
        }

        bool operator<=(const std::nullptr_t) const {
            return (offset <= null_offset);
        }

        bool operator>(const Relative_pointer_impl& ptr) const {
            return (this->operator->() > ptr.operator->());
        }

        bool operator>(const std::nullptr_t) const {
            return (offset > null_offset);
        }

        bool operator<(const Relative_pointer_impl& ptr) const {
            return (this->operator->() < ptr.operator->());
        }

        bool operator<(const std::nullptr_t) const {
            return (offset < null_offset);
        }

        //=================================================
        // Increment/decrement operators
        //=================================================

        Relative_pointer_impl& operator++() {
            offset += delta;
            return *this;
        }

        Relative_pointer_impl operator++(int) {
            offset += delta;
            return {offset - delta};
        }

        Relative_pointer_impl& operator--() {
            offset -= delta;
            return *this;
        }

        Relative_pointer_impl operator--(int) {
            offset -= delta;
            return {offset + delta};
        }

        //=================================================
        // Dereference operators
        //=================================================

        std::conditional_t<is_const, const T&, T&> operator*() const {
            return *(this->operator->());
        }

        absolute_pointer operator->() const {
            auto ptr = (char*)(this);
            auto tmp = offset * stride;
            ptr += tmp;
            return reinterpret_cast<absolute_pointer>(ptr);
        }

        std::conditional_t<is_const, const T&, T&> operator[](const Int n) const {
            auto ptr = (char*)(this);
            ptr += (n + offset) * stride;
            return *reinterpret_cast<absolute_pointer>(ptr);
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
            return (offset != null_offset);
        }

        ///
        /// Conversion to raw pointer
        ///
        /// \return Pointer 
        ///
        explicit operator absolute_pointer() const {
            return this->operator->();
        }

        operator Relative_pointer_impl<T, Int, stride, true>() const {
            return {offset};
        }

        //=================================================
        // Instance members
        //=================================================

        ///
        /// Offset from address of this object
        ///
        Int offset;

    };

    template<class T, class Int, Int stride = 1>
    using Relative_pointer = Relative_pointer_impl<T, Int, stride, false>;

    template<class T, class Int, Int stride = 1>
    using Const_relative_pointer = Relative_pointer_impl<T, Int, stride, true>;

}



namespace std {

    template<class T, class I, I Stride, bool is_const>
    class pointer_traits<aul::Relative_pointer_impl<T, I, Stride, is_const>> {
    public:

        using pointer = aul::Relative_pointer_impl<T, I, Stride, is_const>;
        using elemment_type = T;
        using difference_type = std::make_unsigned<I>;

        template<class U>
        using rebind = aul::Relative_pointer_impl<U, I, Stride, is_const>;

        //TODO: C++20 Make constexpr
        static pointer pointer_to(const T& t) noexcept {
            return pointer{t};
        }

    };

    //TODO: C++ 20 specialize std::to_address

}

#endif //AUL_RELATIVE_POINTER_HPP

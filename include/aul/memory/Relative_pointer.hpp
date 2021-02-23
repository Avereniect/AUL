//
// Created by avereniect on 7/19/20.
//

#ifndef AUL_RELATIVE_POINTER_HPP
#define AUL_RELATIVE_POINTER_HPP

#include "../Bits.hpp"
#include "../Utility.hpp"

#include <type_traits>
#include <memory>

namespace aul {

    ///
    /// A fancy-pointer type which uses relative addressing. A signed integral
    /// value which is an offset added to the this pointer is used to store
    /// the location of the pointed-to object. This means that the pointer can
    /// only point to objects located within the same array in which the
    /// pointer is located. The size of this array can be no larger than
    /// std::numeric_limits<I>::max() / 2 to allow a pointer to the first element
    /// and a pointer to the last element to be subtracted.
    ///
    /// Note, this pointer assumes that the alignment requirements of T and I are
    /// strictly adhered to. That it to say that the pointed to object must
    /// have an alignment of alignof(T) and the Relative_pointer object must
    /// aligned to alignof(I). It is undefined behavior if a pointer which is
    /// not aligned to Stride is used to construct a Relative_pointer object or
    /// if it is assigned to a Relative_pointer object.
    ///
    /// A custom stride value greater than the default of 1 can be used. It
    /// must be a non-zero power of two, no larger than alignof(T).
    ///
    /// Use of #define AUL_INTEGER_POINTERS will potentially speedup certain
    /// operations by treating pointers as integers in certain methods. This is
    /// disabled by default to be standards compliant, but this is safe to do
    /// on most mainstream platforms.
    ///
    /// \tparam T Type pointer points to
    /// \tparam I Backing signed integral type
    /// \tparam S Stride. Must be less than or equal to alignof(T)
    /// \tparam is_const
    template<class T, class I, std::size_t Stride = 1, bool is_const = false>
    class Relative_pointer {

        static_assert(std::is_signed_v<I>);

        static_assert(Stride != 0);
        static_assert(aul::is_pow2(Stride));
        static_assert(Stride <= alignof(T), "Stride is too large to address consecutive elements of type T.");
        static_assert(sizeof(T) % Stride == 0, "Stride must evenly divide sizeof(T).");

        static constexpr I null = (aul::is_aliasing_type_v<T>) ? std::numeric_limits<I>::min() : 0;

        static constexpr bool is_T_void = std::is_same_v<T, void>;

        using raw_pointer = std::conditional_t<is_const, const T*, T*>;

        using reference = std::conditional_t<is_const, const T&, T&>;

    public:

        //=================================================
        // -ctors
        //=================================================

        Relative_pointer() = default;

        ///
        /// Construct pointer null pointer
        ///
        Relative_pointer(const std::nullptr_t):
            offset(null) {}

        ///
        /// \param ptr Primitve pointer to point to. Value is assumed to be an
        /// address aligned to Stride
        ///
        Relative_pointer(const T* ptr):
            offset(compute_offset(ptr)) {}

        Relative_pointer(Relative_pointer&& ptr) noexcept :
            offset((ptr == nullptr) ? null : compute_offset(static_cast<raw_pointer>(ptr))) {}

        Relative_pointer(const Relative_pointer& ptr) :
            offset((ptr == nullptr) ? null : compute_offset(static_cast<raw_pointer>(ptr))) {}

        ~Relative_pointer() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Relative_pointer& operator=(const Relative_pointer& rhs) {
            if (rhs.offset == null) {
                offset = null;
            }

            offset = compute_offset(rhs.compute_raw_pointer());

            return *this;
        }

        Relative_pointer& operator=(Relative_pointer&& rhs) noexcept {
            if (rhs.offset == null) {
                offset = null;
            }

            offset = compute_offset(rhs.compute_raw_pointer());

            return *this;
        }

        Relative_pointer& operator=(const raw_pointer p) {
            offset = compute_offset(p);
            return *this;
        }

        Relative_pointer& operator=(const std::nullptr_t) {
            offset = null;
            return *this;
        }

        //=================================================
        // Arithmetic assignment operators
        //=================================================

        template<class = typename std::enable_if_t<!is_T_void>>
        Relative_pointer& operator+=(const I o) {
            offset += o * sizeof(T) / Stride;
            return *this;
        }

        template<class = typename std::enable_if_t<!is_T_void>>
        Relative_pointer& operator-=(const I o) {
            offset -= o * sizeof(T) / Stride;
            return *this;
        }

        //=================================================
        // Arithmetic operators
        //=================================================

        template<class = typename std::enable_if_t<!is_T_void>>
        Relative_pointer operator+(const I o) const {
            auto ret = *this;
            ret.offset += o * sizeof(T) / Stride;
            return ret;
        }

        template<class = typename std::enable_if_t<!is_T_void>>
        friend Relative_pointer operator+(const I o, const Relative_pointer& ptr) {
            return ptr + o * sizeof(T) / Stride;
        }

        template<class = typename std::enable_if_t<!is_T_void>>
        Relative_pointer operator-(const I o) const {
            auto ret = *this;
            ret.offset -= o * sizeof(T) / Stride;
            return ret;
        }

        template<class = typename std::enable_if_t<!is_T_void>>
        I operator-(const Relative_pointer& rhs) {
            return compute_raw_pointer() - rhs.compute_raw_pointer();
        }

        //=================================================
        // Comparison operators
        //=================================================

        bool operator==(const Relative_pointer& rhs) const {
            return (compute_raw_pointer() == rhs.compute_raw_pointer());
        }

        bool operator==(const std::nullptr_t) const {
            return (offset == null);
        }

        bool operator!=(const Relative_pointer& rhs) const {
            return (compute_raw_pointer() != rhs.compute_raw_pointer());
        }

        bool operator!=(const std::nullptr_t) const {
            return (offset != null);
        }

        bool operator>=(const Relative_pointer& rhs) const {
            return (compute_raw_pointer() >= rhs.compute_raw_pointer());
        }

        bool operator<=(const Relative_pointer& rhs) const {
            return (compute_raw_pointer() <= rhs.compute_raw_pointer());
        }

        bool operator>(const Relative_pointer& rhs) const {
            return (compute_raw_pointer() > rhs.compute_raw_pointer());
        }

        bool operator<(const Relative_pointer& rhs) const {
            return (compute_raw_pointer() < rhs.compute_raw_pointer());
        }

        //=================================================
        // Increment/decrement operators
        //=================================================

        template<class = typename std::enable_if_t<!is_T_void>>
        Relative_pointer& operator++() {
            offset += (sizeof(T) / Stride);
            return *this;
        }

        template<class = typename std::enable_if_t<!is_T_void>>
        raw_pointer operator++(int) {
            raw_pointer ret = compute_raw_pointer();
            offset += (sizeof(T) / Stride);
            return ret;
        }

        template<class = typename std::enable_if_t<!is_T_void>>
        Relative_pointer& operator--() {
            offset -= (sizeof(T) / Stride);
            return *this;
        }

        template<class = typename std::enable_if_t<!is_T_void>>
        raw_pointer operator--(int) {
            raw_pointer ret = compute_raw_pointer();
            offset -= (sizeof(T) / Stride);
            return ret;
        }

        //=================================================
        // Dereference operators
        //=================================================

        template<class = typename std::enable_if_t<!is_T_void>>
        reference operator*() const {
            return *compute_raw_pointer();
        }

        template<class = typename std::enable_if_t<!is_T_void>>
        raw_pointer operator->() const {
            return compute_raw_pointer();
        }

        template<class = typename std::enable_if_t<!is_T_void>>
        reference operator[](const I x) const {
            raw_pointer temp = compute_raw_pointer();
            return temp[x];
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
            return (offset != null);
        }

        ///
        /// Conversion to raw pointer
        ///
        /// \return Pointer 
        ///
        explicit operator raw_pointer() const {
            return compute_raw_pointer();
        }

    private:

        //=================================================
        // Instance members
        //=================================================

        ///
        /// Offset from address of this object
        ///
        I offset;

        //=================================================
        // Helper functions
        //=================================================

        I compute_offset(const T* ptr) const {
            std::ptrdiff_t raw_diff = ((unsigned char*)(ptr) - (unsigned char*)(this));

            int x = raw_diff;

            I raw_diff_whole = raw_diff / static_cast<signed>(Stride);
            I raw_diff_frac = (raw_diff - (raw_diff_whole * Stride));

            I diff = raw_diff_whole + ((raw_diff > 0) && raw_diff_frac);

            return diff;
        }

        raw_pointer compute_raw_pointer() const {
            if (offset == null) {
                return nullptr;
            }

            if constexpr (0 == (alignof(T) - alignof(I)) % Stride ) { //Byte offset is always an integer multiple of Stride
                auto p0 = (unsigned char*)(this); //Casting away const!!!
                I temp = offset * Stride;
                return reinterpret_cast<raw_pointer>(p0 + temp);
            } else { // Byte offset may not be integer multiple of Stride. Alignment required

                #ifdef AUL_INTEGER_POINTERS

                auto p_int = reinterpret_cast<std::make_signed_t<std::intptr_t>>(this);

                constexpr std::intptr_t mask = ~static_cast<std::intptr_t>(Stride - 1);
                p_int &= mask; //Rounds address down to multiple of Stride

                std::ptrdiff_t delta = static_cast<std::ptrdiff_t>(offset) * static_cast<std::ptrdiff_t>(Stride);
                p_int += delta;

                return reinterpret_cast<raw_pointer>(p_int);

                #else

                // Const is cast away because std::aligned requires a non-const pointer
                // Pointer is never dereferenced however.
                auto p0 = (unsigned char*)(this);

                if (0 < offset) {
                    std::size_t size = offset * Stride + sizeof(T);

                    void* param_ptr = reinterpret_cast<void*>(p0);
                    auto* aligned_temp = reinterpret_cast<unsigned char*>(std::align(Stride, sizeof(char), param_ptr, size));
                    if (aligned_temp == p0) {
                        return reinterpret_cast<raw_pointer>(aligned_temp + (offset * Stride));
                    } else {
                        return reinterpret_cast<raw_pointer>(aligned_temp + ((offset - 1) * Stride));
                    }
                } else {
                    std::size_t size = offset * Stride;
                    void* param_ptr = reinterpret_cast<void*>(p0 + 1);

                    auto* aligned_ptr = reinterpret_cast<unsigned char*>(std::align(Stride, sizeof(char), param_ptr, size));

                    return reinterpret_cast<raw_pointer>(aligned_ptr + (offset - 1) * Stride);
                }

                #endif
            }
        }

    };

    template<class T, class I, unsigned Stride = 1>
    using Const_relative_pointer = Relative_pointer<T, I, Stride, true>;

}



namespace std {

    template<class T, class I, std::size_t Stride, bool is_const>
    class pointer_traits<aul::Relative_pointer<T, I, Stride, is_const>> {
    public:

        using pointer = aul::Relative_pointer<T, I, Stride, is_const>;
        using element_type = T;
        using difference_type = I;

        template<class U>
        using rebind = aul::Relative_pointer<U, I, Stride, is_const>;

        //TODO: C++20 Make constexpr
        static pointer pointer_to(const T& t) noexcept {
            return pointer{t};
        }

    };

    //TODO: C++ 20 specialize std::to_address

}

#endif //AUL_RELATIVE_POINTER_HPP

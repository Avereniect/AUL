#ifndef AUL_BIT_FIELD_ITERATOR_HPP
#define AUL_BIT_FIELD_ITERATOR_HPP

#include "../Bits.hpp"

#include <memory>
#include <cstdint>
#include <climits>

namespace aul {

    ///
    /// A custom reference type which points to an integral element consisting
    /// of a contiguous set of bits anywhere in memory.
    ///
    /// \tparam is_const Indicates mutability of interface
    /// \tparam A Allocator type
    template<class T, class P = T*>
    class Bit_field_ref {
        static constexpr unsigned short bits_per_element = sizeof(T) * CHAR_BIT;
    public:

        static_assert(std::is_integral_v<T>);
        static_assert(std::is_same_v<T, typename std::pointer_traits<P>::element_type>);

        //=================================================
        // Type alias
        //=================================================

        using value_type = T;
        using pointer = P;

        //=================================================
        // -ctors
        //=================================================

        Bit_field_ref(pointer ptr, const unsigned char offset, const unsigned char num_bits):
            ptr(ptr),
            offset(offset),
            size(num_bits) {}

        ~Bit_field_ref() = default;

        //=================================================
        // Assignment operators
        //=================================================

        template<class = typename std::enable_if<!std::is_const_v<T>>::type>
        Bit_field_ref& operator=(const value_type v) {
            if (bits_per_element < offset + size) {
                //Write to two value_types

                ptr[0] &= (value_type(-1) << offset);
                ptr[0] |= (v << offset);

                const value_type overhang = (offset + size - bits_per_element);
                ptr[1] &= ~(value_type(-1) << overhang);
                ptr[1] |= (v >> overhang);

            } else {
                //Write to single value_type

                value_type mask = fill_bits<T>(offset, offset + size);

                value_type tmp = *ptr;
                tmp &= ~mask; //Clear existing content
                tmp |= (mask & (v << offset));

                *ptr = tmp;
            }

            return *this;
        }

        //=================================================
        // Conversion operators
        //=================================================

        operator value_type() const {
            if (bits_per_element < offset + size) {
                //Read from two value_type addresses
                auto a_bits = (bits_per_element - offset);
                auto b_bits = size - (a_bits);

                T mask_a = fill_first_n_bits<T>(a_bits);
                T data_a = (ptr[0] >> offset);

                T mask_b = fill_first_n_bits<T>(b_bits);
                T data_b = ptr[1];

                return (mask_a & data_a) | ((mask_b & data_b) << a_bits);
            } else {
                //Read from single value_type address
                T data = (*ptr >> offset);
                T mask = fill_first_n_bits<T>(size); //(~value_type{0} >> (bits_per_element - size)) << offset;
                return data & mask;
            }
        }

    private:

        //=================================================
        // Instance variables
        //=================================================

        /// Pointer to first value_type element that contains a bit from the bit field
        pointer ptr = nullptr;

        /// Index of bit field's first bit within *ptr
        unsigned char offset = 0;

        /// Number of bits in bit field
        unsigned char size = 0;

    };





    ///
    /// TODO: Class could benefit from use of BMI and BMI2 instruction sets.
    ///
    /// \tparam is_const Whether iterator is to constant object
    /// \tparam A Allocator type
    template<class T, class P = T*, class D = typename std::pointer_traits<P>::difference_type>
    class Bit_field_iterator {
    public:

        static_assert(std::is_integral_v<T>);
        static_assert(std::is_same_v<T, typename std::pointer_traits<P>::element_type>);
        static_assert(std::numeric_limits<D>::digits >= std::numeric_limits<typename std::pointer_traits<P>::difference_type>::digits);

        //=================================================
        // Type aliases
        //=================================================

        using value_type = T;
        using pointer = P;
        using reference = Bit_field_ref<T, P>;
        using difference_type = D;
        using iterator_category = std::random_access_iterator_tag;

    private:

        static constexpr std::size_t bits_per_element = sizeof(value_type) * CHAR_BIT;

    public:

        //=================================================
        // -ctors
        //=================================================

        Bit_field_iterator(pointer ptr, const unsigned short offset, const unsigned short size):
            ptr(ptr),
            offset(offset),
            size(size) {}

        Bit_field_iterator() = default;
        Bit_field_iterator(const Bit_field_iterator&) = default;
        Bit_field_iterator(Bit_field_iterator&&) noexcept = default;
        ~Bit_field_iterator() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Bit_field_iterator& operator=(const Bit_field_iterator&) = default;
        Bit_field_iterator& operator=(Bit_field_iterator&&) noexcept = default;

        //=================================================
        // Comparison operators
        //=================================================

        bool operator==(Bit_field_iterator& rhs) const {
            return
                ptr == rhs.ptr &&
                offset == rhs.offset &&
                size == rhs.size;
        }

        bool operator!=(Bit_field_iterator& rhs) const {
            return
                ptr != rhs.ptr ||
                offset != rhs.offset ||
                size != rhs.size;
        }

        bool operator<=(Bit_field_iterator& rhs) const {
            return
                ptr <= rhs.ptr &&
                offset <= rhs.offset &&
                size <= rhs.size;
        }

        bool operator<(Bit_field_iterator& rhs) const {
            if (ptr < rhs.ptr) {
                return true;
            }
            if (offset < rhs.offset) {
                return true;
            }
        }

        bool operator>=(Bit_field_iterator& rhs) const {
            return
                ptr >= rhs.ptr &&
                offset >= rhs.offset &&
                size >= rhs.size;
        }

        bool operator>(Bit_field_iterator& rhs) const {
            if (ptr > rhs.ptr) {
                return true;
            }
            if (offset > rhs.offset) {
                return true;
            }
            return false;
        }

        //=================================================
        // Arithmetic assignment operators
        //=================================================

        Bit_field_iterator& operator+=(const difference_type o) {
            // o = a + b
            difference_type b = o % CHAR_BIT;
            difference_type a = o - b;

            // a = x * bits_per_element + y;
            difference_type x = a / bits_per_element;
            difference_type y = a % bits_per_element;

            static_assert(sizeof(std::ptrdiff_t) >= sizeof(T));
            // This could still technically overflow if std::ptrdiff_t is not at
            // least as large as bits_per_element, as unlikely as that would be.
            std::ptrdiff_t partial = size * y + size * b;
            std::ptrdiff_t whole   = size * x;

            partial += offset;
            whole  += (partial / ptrdiff_t(bits_per_element));
            partial = (partial % ptrdiff_t(bits_per_element));

            ptr += whole;
            offset = partial;
        }

        Bit_field_iterator& operator-=(const difference_type o) {
            *this += -o;
            return *this;
        }

        //=================================================
        // Increment/Decrement operators
        //=================================================

        Bit_field_iterator& operator++() {
            bool higher_address = (bits_per_element <= (offset + size));

            offset += (offset + size) - (higher_address ? bits_per_element : 0);
            ptr += higher_address;

            return *this;
        }

        Bit_field_iterator operator++(int) {
            auto ret = *this;
            ++(*this);
            return ret;
        }

        Bit_field_iterator& operator--() {
            bool lower_address = (size > offset);
            ptr -= lower_address;
            offset = (offset - size) + (lower_address ? bits_per_element : 0);

            return *this;
        }

        Bit_field_iterator operator--(int) {
            auto ret = *this;
            --(*this);
            return ret;
        }

        //=================================================
        // Arithmetic operators
        //=================================================

        Bit_field_iterator operator+(const difference_type x) const {
            Bit_field_iterator ret = *this;
            ret += x;
            return ret;
        }

        Bit_field_iterator operator-(const difference_type x) const {
            Bit_field_iterator ret = *this;
            ret -= x;
            return ret;
        }

        //=================================================
        // Dereference operators
        //=================================================

        reference operator[](const difference_type n) {
            return *(this + n);
        }

        reference operator[](const difference_type n) const {
            return *(this + n);
        }

        reference operator*() {
            return reference{ptr, offset, size};
        }

        reference operator*() const {
            return reference{ptr, offset, size};
        }

    private:

        /// Pointer to element containing first bit of bit field
        pointer ptr;

        /// Number of bits into *ptr which the bit field begins
        unsigned short offset{};

        /// Number of bits in the bit field
        unsigned short size{};

    };



}

#endif //AUL_BIT_FIELD_ITERATOR_HPP

#ifndef AUL_DRLE_RANGE_HPP
#define AUL_DRLE_RANGE_HPP

#include <cstdint>
#include <memory>
#include <vector>
#include <utility>

#include "Algorithms.hpp"

namespace aul {

    ///
    /// A class representing a subrange of a larger range represented by an
    /// instance of the DRLE_range class.
    ///
    /// This is an implementation detail. This class should not be used directly
    /// by users of the AUL library.
    ///
    /// \tparam T Type of subrange elements. Must be an integral type
    template<class T>
    struct DRLE_subrange {

        using slope_type = typename std::make_signed<T>::type;

        DRLE_subrange(
            T initial,
            T slope,
            bool is_slope_inverted,
            std::size_t size,
            std::size_t initial_index
        ) :
            initial(initial),
            slope(slope),
            is_slope_inverted(is_slope_inverted),
            size(size),
            initial_index(initial_index) {}

        DRLE_subrange() = default;
        DRLE_subrange(const DRLE_subrange&) = default;
        DRLE_subrange(DRLE_subrange&&) noexcept = default;
        ~DRLE_subrange() = default;

        T initial;
        slope_type slope;
        bool is_slope_inverted;
        slope_type size;
        std::size_t initial_index;
    };



    ///
    ///
    ///
    /// \tparam T Type of elements in range
    template<class T>
    class DRLE_range_iterator{
    public:

        //=================================================
        // -ctors
        //=================================================

        DRLE_range_iterator(const DRLE_subrange<T>* subrange, std::ptrdiff_t o):
            ptr(subrange),
            offset(o) {}

        DRLE_range_iterator() = default;
        DRLE_range_iterator(const DRLE_range_iterator&) = default;
        DRLE_range_iterator(DRLE_range_iterator&&) noexcept = default;
        ~DRLE_range_iterator() = default;

        //=================================================
        // Assignment Operators
        //=================================================

        DRLE_range_iterator& operator=(const DRLE_range_iterator&) = default;
        DRLE_range_iterator& operator=(DRLE_range_iterator&&) = default;

        //=================================================
        // Comparison operators
        //=================================================

        friend bool operator==(DRLE_range_iterator lhs, DRLE_range_iterator rhs) {
            return (lhs.ptr == rhs.ptr) && (lhs.offset == rhs.offset);
        }

        friend bool operator!=(DRLE_range_iterator lhs, DRLE_range_iterator rhs) {
            return (lhs.ptr != rhs.ptr) || (lhs.offset != rhs.offset);
        }

        friend bool operator<(DRLE_range_iterator lhs, DRLE_range_iterator rhs) {
            if (lhs.ptr < rhs.ptr) {
                return true;
            }

            return (lhs.ptr == rhs.ptr) && (lhs.offset < rhs.offset);
        }

        friend bool operator<=(DRLE_range_iterator lhs, DRLE_range_iterator rhs) {
            if (lhs.ptr < rhs.ptr) {
                return true;
            }

            return (lhs.ptr == rhs.ptr) && (lhs.offset <= rhs.offset);
        }

        friend bool operator>(DRLE_range_iterator lhs, DRLE_range_iterator rhs) {
            if (lhs.ptr > rhs.ptr) {
                return true;
            }

            return (lhs.ptr == lhs.ptr) && (lhs.offset > rhs.offset);
        }

        friend bool operator>=(DRLE_range_iterator lhs, DRLE_range_iterator rhs) {
            if (lhs.ptr > rhs.ptr) {
                return true;
            }

            return (lhs.ptr == lhs.ptr) && (lhs.offset < rhs.offset);
        }

        //=================================================
        // Increment operators
        //=================================================

        DRLE_range_iterator& operator++() {
            ++offset;
            if (ptr->size == offset) {
                ++ptr;
                offset = 0;
            }

            return *this;
        }

        DRLE_range_iterator operator++(int) {
            auto tmp = *this;
            ++offset;
            if (ptr->size == offset) {
                ++ptr;
                offset = 0;
            }
            return tmp;
        }

        DRLE_range_iterator& operator--() {
            if (offset == 0) {
                --ptr;
                offset = ptr->size - 1;
            } else {
                --offset;
            }

            return *this;
        }

        DRLE_range_iterator operator--(int) {
            auto tmp = *this;
            if (offset == 0) {
                --ptr;
                offset = ptr->size - 1;
            } else {
                --offset;
            }
            return tmp;
        }

        //=================================================
        // Arithmetic Assignment Operators
        //=================================================

        DRLE_range_iterator& operator+=(std::ptrdiff_t o) {
            if (o < 0) {
                *this -= -o;
                return *this;
            }

            while (true) {
                auto remaining_in_subrange = (ptr->size) - offset;
                if (o < remaining_in_subrange) {
                    offset = o;
                    break;
                }

                ++ptr;
                offset = 0;
                o -= remaining_in_subrange;
            }

            return *this;
        }

        DRLE_range_iterator& operator-=(std::ptrdiff_t o) {
            if (o < 0) {
                *this += -o;
                return *this;
            }

            while (true) {
                auto remaining_in_subrange = offset;
                if (o <= remaining_in_subrange) {
                    offset -= o;
                    break;
                }

                --ptr;
                offset = ptr->size;
                o -= remaining_in_subrange;
            }

            return *this;
        }

        //=================================================
        // Arithmetic Operators
        //=================================================

        friend DRLE_range_iterator operator+(DRLE_range_iterator lhs, std::ptrdiff_t rhs) {
            lhs += rhs;
            return lhs;
        }

        friend DRLE_range_iterator operator+(std::ptrdiff_t lhs, DRLE_range_iterator rhs) {
            rhs += lhs;
            return rhs;
        }

        friend std::ptrdiff_t operator-(DRLE_range_iterator lhs, DRLE_range_iterator rhs) {
            std::ptrdiff_t sum = 0;

            if (lhs.ptr != rhs.ptr) {
                sum += lhs.offset;
                lhs.offset = 0;
                ++lhs;
            }

            for (; lhs != rhs; ++lhs) {
                sum += lhs.ptr->size;
            }

            sum += (lhs.offset - rhs.ptr);

            return sum;
        }

        //=================================================
        // Dereference Operators
        //=================================================

        T operator*() const {
            if (ptr->is_slope_inverted) {
                auto a = ptr->initial;
                auto b = (offset / ptr->slope);
                auto c = a + b;
                return c;
            } else {
                return ptr->initial + (offset * ptr->slope);
            }
        }

        T operator[](std::size_t o) const {
            auto tmp = *this;
            tmp += o;
            return *tmp;
        }

    private:

        //=================================================
        // Instance Members
        //=================================================

        const DRLE_subrange<T>* ptr = nullptr;
        std::ptrdiff_t offset = 0;

    };

    ///
    /// A class representing a sequence of integers using a combination of
    /// delta encoding and run-length encoding.
    ///
    /// TODO: Allow for customization of types used in subrange representation
    ///
    /// \tparam T Type of objects to compress. Should be an integral type
    /// \tparam A Allocator
    template<class T, class A = std::allocator<T>>
    class DRLE_range {
        static_assert(
            std::is_integral<T>::value,
            "T is required to be an integral type"
        );

    public:

        //=================================================
        // Type aliases
        //=================================================

        using value_type = T;

        using reference = const T&;
        using const_reference = const T&;

        using pointer = typename std::allocator_traits<A>::const_pointer;
        using const_pointer = typename std::allocator_traits<A>::const_pointer;

        using size_type = typename std::allocator_traits<A>::size_type;
        using difference_type = typename std::allocator_traits<A>::difference_type;

        using iterator = DRLE_range_iterator<T>;
        using const_iterator = iterator;

        using slope_type = typename std::make_signed<T>::type;

        //=================================================
        // Helper types
        //=================================================

    private:

        struct Constructor_helper {

            Constructor_helper(
                std::vector<DRLE_subrange<T>, A>&& subranges,
                size_type range_size
            ):
                subranges(subranges),
                range_size(range_size) {}

            std::vector<DRLE_subrange<T>, A> subranges;
            size_type range_size;
        };

    public:

        //=================================================
        // Constructors
        //=================================================

        template<class It>
        DRLE_range(It begin, It end):
            DRLE_range(compress(begin, end)) {}

    private:

        explicit DRLE_range(Constructor_helper&& helper):
            subranges(std::move(helper.subranges)),
            range_size(helper.range_size) {}

    public:

        DRLE_range() = default;

        DRLE_range(const DRLE_range& other):
            subranges(other.subranges),
            range_size(other.subranges) {}

        DRLE_range(DRLE_range&& other) noexcept:
            subranges(std::exchange(other.subranges, {})),
            range_size(std::exchange(other.range_size, 0)) {}

        ~DRLE_range() = default;

        //=================================================
        // Assignment operators
        //=================================================

        DRLE_range& operator=(const DRLE_range& rhs) {
            subranges = rhs.subranges;
            range_size = rhs.range_size;

            return *this;
        }

        ///
        /// \param rhs Object to move from
        /// \return Reference to *this
        DRLE_range& operator=(DRLE_range&& rhs) noexcept {
            subranges = std::exchange(rhs.subranges, {});
            range_size = std::exchange(rhs.range_size, 0);

            return *this;
        }

        //=================================================
        // Iterator Methods
        //=================================================

        iterator begin() const {
            return iterator{&subranges.front(), 0};
        }

        iterator cbegin() const {
            return const_cast<const DRLE_range&>(*this).begin();
        }

        iterator end() const {
            return iterator{&subranges.back(), subranges.back().size};
        }

        iterator cend() const {
            return const_cast<const DRLE_range&>(*this).end();
        }

        //=================================================
        // Element accessors
        //=================================================

        ///
        /// \param i Index of value to retrieve
        /// \return Copy of value at i'th index
        [[nodiscard]]
        T operator[](size_type i) const {
            auto it = aul::binary_search(subranges.begin(), subranges.end(), i, comparator);
            if (it == subranges.end()) {
                it = subranges.end() - 1;
            }

            if (it->initial_index > i) {
                --it;
            }

            T reconstructed_value = it->initial;
            if (it->is_slope_inverted) {
                reconstructed_value += static_cast<slope_type>(i - it->initial_index) / it->slope;
            } else {
                reconstructed_value += static_cast<slope_type>(i - it->initial_index) * it->slope;
            }

            return reconstructed_value;
        }

        //=================================================
        // Accessors
        //=================================================

        ///
        /// \return The number of elements in the compressed format
        [[nodiscard]]
        size_type size() const {
            return range_size;
        }

        ///
        /// \return True if size() == 0
        [[nodiscard]]
        bool empty() const {
            return subranges.empty();
        }

        //=================================================
        // Mutators
        //=================================================

        ///
        /// Clears the contents of the current RLE range
        ///
        void clear() {
            subranges.clear();
            range_size = 0;
        }

    private:

        //=================================================
        // Static members
        //=================================================

        static bool comparator(const DRLE_subrange<T>& a, T b) {
            return (a.initial_index < b);
        }

        //=================================================
        // Instance members
        //=================================================

        std::vector<DRLE_subrange<T>, A> subranges;
        size_type range_size = 0;

        //=================================================
        // Helper functions
        //=================================================

        ///
        /// Helper function that performs the compression algorithm
        ///
        /// \tparam It Forward iterator type
        /// \param a Iterator to beginning of range
        /// \param b Iterator to end of range
        /// \return Struct containing compressed subranges and subrange size
        template<class It>
        Constructor_helper compress(It a, It b) {
            std::vector<DRLE_subrange<T>, A> ret;

            size_type range_size = 0;

            // Return early if the input range is empty
            if (a == b) {
                return {std::move(ret), range_size};
            }

            slope_type slope = 0;
            bool is_slope_inverted = false;
            value_type subrange_initial = *a;
            difference_type subrange_size = 0;

            ++a;
            ++range_size;
            ++subrange_size;

            // Iterate over input range
            for (; a != b; ++a, ++range_size, ++subrange_size) {
                // Predict next value based on current slope
                T predicted_value = subrange_initial;
                if (is_slope_inverted) {
                    slope_type y = subrange_size / slope;
                    predicted_value += y;
                } else {
                    slope_type y = subrange_size * slope;
                    predicted_value += y;
                }

                value_type observed_value = *a;

                // Handle case where observed value doesn't match predicted value
                if (predicted_value != observed_value) {
                    // Compute slope of new subrange if this element is a new range
                    if (slope == 0 && subrange_size == 1) {
                        slope = (*a - subrange_initial);
                        continue;
                    }

                    using std::abs;

                    slope_type difference = (*a - subrange_initial);
                    // Handle case where the slope of the new subrange is less than 1
                    auto c0 = slope == 0;
                    auto c1 = subrange_size != 0;
                    auto c2 = 1 == abs(static_cast<slope_type>(difference));

                    auto c3 = c0 && c1 && c2;

                    if (c3) {
                        slope = (difference > 0) ? subrange_size : -subrange_size;
                        is_slope_inverted = true;
                        continue;
                    }

                    ret.emplace_back(
                        subrange_initial,
                        slope,
                        is_slope_inverted,
                        subrange_size,
                        static_cast<size_type>(range_size - subrange_size)
                    );

                    slope = 0;
                    is_slope_inverted = false;
                    subrange_initial = *a;
                    subrange_size = 0;
                }
            }

            ret.emplace_back(
                subrange_initial,
                slope,
                is_slope_inverted,
                subrange_size,
                static_cast<std::size_t>(range_size - subrange_size)
            );

            return Constructor_helper{std::move(ret), range_size};
        }

    };

}

#endif //AUL_DRLE_RANGE_HPP

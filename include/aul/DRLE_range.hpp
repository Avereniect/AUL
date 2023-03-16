#ifndef AUL_DRLE_RANGE_HPP
#define AUL_DRLE_RANGE_HPP

#include <cstdint>
#include <memory>
#include <vector>
#include <utility>

#include "Algorithms.hpp"

namespace aul {

    // TODO: Implement
    template<class T, class P>
    class DRLE_range_iterator;

    ///
    /// A class representing a derivative run-length encoded range of integers.
    ///
    ///
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

        //=================================================
        // Helper types
        //=================================================

    private:

        struct DRLE_subrange {

            DRLE_subrange(
                T initial,
                T slope,
                bool is_slope_inverted,
                size_type size,
                size_type initial_index
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
            T slope;
            bool is_slope_inverted;
            size_type size;
            size_type initial_index;
        };

        struct Constructor_helper {

            Constructor_helper(
                std::vector<DRLE_subrange, A>&& subranges,
                size_type range_size
            ):
                subranges(subranges),
                range_size(range_size) {}

            std::vector<DRLE_subrange, A> subranges;
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
                reconstructed_value += (i - it->initial_index) / it->slope;
            } else {
                reconstructed_value += (i - it->initial_index) * it->slope;
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

        static bool comparator(const DRLE_subrange& a, T b) {
            return (a.initial_index < b);
        }

        //=================================================
        // Instance members
        //=================================================

        std::vector<DRLE_subrange, A> subranges;
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
            std::vector<DRLE_subrange, A> ret;

            size_type range_size = 0;

            if (a == b) {
                return {std::move(ret), range_size};
            }

            value_type slope = 0;
            bool is_slope_inverted = false;
            value_type subrange_initial = *a;
            size_type subrange_size = 0;

            ++a;
            ++range_size;
            ++subrange_size;

            for (; a != b; ++a, ++range_size, ++subrange_size) {
                T predicted_value;
                if (is_slope_inverted) {
                    predicted_value = subrange_size / slope + subrange_initial;
                } else {
                    predicted_value = subrange_size * slope + subrange_initial;
                }

                value_type observed_value = *a;

                if (predicted_value != observed_value) {
                    if (slope == 0 && subrange_size == 1) {
                        slope = (*a - subrange_initial);
                        continue;
                    }

                    difference_type difference = *a - subrange_initial;
                    if (slope == 0 && subrange_size != 0 && (1 == std::abs(difference))) {
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

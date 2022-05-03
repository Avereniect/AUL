//
// Created by avereniect on 2/15/22.
//

#ifndef AUL_RLE_RANGE_HPP
#define AUL_RLE_RANGE_HPP

#include <cstdint>
#include <memory>
#include <vector>

#include "Algorithms.hpp"

namespace aul {

    template<class T, class A = std::allocator<T>>
    class RLE_range {
    public:

        //=================================================
        // Type aliases
        //=================================================

        using value_type = T;

        using refernece = const T&;
        using const_reference = const T&;

        using pointer = typename std::allocator_traits<A>::const_pointer;
        using const_pointer = typename std::allocator_traits<A>::const_pointer;

        using size_type = typename std::allocator_traits<A>::size_type;
        using difference_type = typename std::allocator_traits<A>::difference_type;

        //=================================================
        // Helper types
        //=================================================

        struct RLE_subrange {
            T start;
            T derivative;
            size_type length;
            size_type start_index;
        };

        //=================================================
        // Constructors
        //=================================================

        template<class It>
        RLE_range(It begin, It end):
            subranges(compress(begin, end)) {}

        RLE_range() {}
        ~RLE_range() {};

        //=================================================
        // Assignment operators
        //=================================================

        //=================================================
        // Element accessors
        //=================================================

        T operator[](size_type i) const {
            auto it = aul::binary_search(subranges.begin(), subranges.end(), comparator);
            return it->begin + (i - it->start_index) * it->derivative;
        }

        //=================================================
        // Accessors
        //=================================================

        size_type size() const {
            if (subranges.empty()) {
                return 0;
            }

            auto& back = subranges.back();

            return back.first_index + back.length;
        }

        bool empty() const {
            return subranges.empty();
        }

    private:

        //=================================================
        // Static members
        //=================================================

        static auto comparator = [] (T a, const RLE_subrange& b) {
            return (a < b.start_index);
        };

        //=================================================
        // Instance members
        //=================================================

        std::vector<RLE_subrange, A> subranges;

        //=================================================
        // Helper functions
        //=================================================

        template<class It>
        std::vector<RLE_subrange, A> compress(It a, It b) {
            std::vector<RLE_subrange, A> ret{};

            T last = *a;
            size_type length = 1;
            while (a != b) {
                ++a;
                T curr_dx = (*a - last);
                while (a != b ) {

                }

                length = 0;
            }

            return ret;
        }

        ///
        /// \tparam It Forward iterator type
        /// \param a Iterator to start of range
        /// \param b Iterator to end of range
        /// \return Iterator one past last element in range to have the same derivative
        template<class It>
        It subrange_end(It begin, It end) {
            if (end - begin == 0) {
                return RLE_subrange{};
            }

            T a = begin;
            T b = ++a;

            T dx = (*b - *a);
            while ((b != end) && ((*b - *a) == dx)) {
                ++a;
                ++b;

            }

            return b;
        }

    };

}

#endif //AUL_RLE_RANGE_HPP

//
// Created by avereniect on 1/11/22.
//

#ifndef AUL_MULTISPAN_HPP
#define AUL_MULTISPAN_HPP

//#include "Zipperator.hpp"

#include <cstdint>
#include <type_traits>

namespace aul {

    ///
    /// Constant which if passed to aul::Span<T, E> as E indicates that the
    /// extent isn't known at compile time.
    ///
    static constexpr std::size_t dynamic_extent = -1;

    ///
    /// Base class for span classes.
    ///
    /// \tparam E Span extent
    template<std::size_t E>
    class Span_base {
    protected:

        Span_base() = default;

        explicit Span_base(std::size_t n) {}

        std::size_t size() const {
            return E;
        }

    };

    ///
    /// Specialization of Span_base for spans of a dynamic length
    ///
    template<>
    class Span_base<dynamic_extent> {
    protected:

        Span_base():
            n(0) {}

        explicit Span_base(std::size_t n):
            n(n) {}

        std::size_t size() const {
            return n;
        }

        std::size_t n;

    };

    ///
    /// \tparam T Type of element viewed by span
    /// \tparam E Extent of span
    template<class T, std::size_t E = dynamic_extent>
    class Span : Span_base<E>{
        using base = Span_base<E>;
    public:

        static constexpr std::size_t extent = dynamic_extent;

        //=================================================
        // Type aliases
        //=================================================

        using element_type = T;
        using value_type = std::remove_cv_t<T>;

        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        using pointer = T*;
        using const_pointer = const T*;

        using reference = T&;
        using const_reference = const T&;

        using iterator = pointer; //Random_access_zipperator<pointer>;
        using reverse_iterator = std::reverse_iterator<iterator>;

        //=================================================
        // -ctors
        //=================================================

        ///
        /// \tparam It Iterator type
        /// \param first Iterator to first element in
        /// \param count
        template<class It, class = typename std::enable_if_t<E == dynamic_extent, It>>
        Span(It first, size_type count):
            base(count),
            ptr(std::addressof(*first)) {}

        ///
        /// \tparam It
        /// \param first
        template<class It, class = typename std::enable_if_t<E != dynamic_extent, It>>
        explicit Span(It first):
            base(),
            ptr(std::addressof(*first)) {}

        ///
        /// \tparam It
        /// \tparam End
        /// \param first
        /// \param end
        template<class It, class End, class = typename std::enable_if_t<E == dynamic_extent, It>>
        Span(It first, End end):
            base(std::addressof(*end) - std::addressof(*first)),
            ptr(std::addressof(*first)) {}

        ///
        /// \tparam N Length of primitive array.
        /// \param arr Primitive array to span over
        template<std::size_t N, class = typename std::enable_if_t<E == dynamic_extent || E <= N>>
        explicit Span(element_type (&arr)[N]) noexcept:
            base(N),
            ptr(std::data(arr)) {}

        ///
        /// \tparam N Length of std::array
        /// \param arr std::array object to span over
        template<std::size_t N, class = typename std::enable_if_t<E == dynamic_extent || E <= N>>
        explicit Span(std::array<element_type, N>& arr) noexcept:
            base(N),
            ptr(std::data(arr)) {}

        ///
        /// \tparam N
        /// \param arr
        template<std::size_t N, class = typename std::enable_if_t<E == dynamic_extent || E <= N>>
        explicit Span(const std::array<std::remove_const_t<element_type>, N>& arr) noexcept:
            base(N),
            ptr(std::data(arr)) {}

        Span() = default;
        Span(const Span&) = default;
        Span(Span&&) noexcept = default;
        ~Span() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Span& operator=(const Span&) = default;
        Span& operator=(Span&&) noexcept = default;

        //=================================================
        // Iterator methods
        //=================================================

        iterator begin() const {
            return iterator{ptr};
        }

        iterator end() const {
            return iterator{ptr + size()};
        }

        //=================================================
        // Element accessors
        //=================================================

        reference front() const {
            return ptr[0];
        }

        reference back() const {
            return ptr[size() - 1];
        }

        reference operator[](size_type idx) const {
            return ptr[idx];
        }

        pointer data() const {
            return ptr;
        }

        //=================================================
        // Accessors
        //=================================================

        std::size_t size() const {
            return base::size();
        }

        std::size_t size_bytes() const {
            return size() * sizeof(element_type);
        }

        bool empty() const {
            return size() == 0;
        }

        //=================================================
        // Subviews
        //=================================================

        template<std::size_t Count>
        Span<element_type, Count> first() const {
            Span<element_type, Count> ret;
            ret.ptr = ptr;
            return ret;
        }

        Span<element_type, dynamic_extent> first(std::size_t c) const {
            return Span<element_type, dynamic_extent>{ptr, c};
        }

        template<std::size_t Offset, std::size_t Count = dynamic_extent>
        Span<element_type, (Count == dynamic_extent) ? dynamic_extent : (E != dynamic_extent) ? (E - Count) : dynamic_extent>
        subspan() const {
            return {};
        }

        //=================================================
        // Conversion operators
        //=================================================

        operator Span<const T, E>() const {
            return {ptr, size()};
        };

    private:

        //=================================================
        // Instance members
        //=================================================

        pointer ptr = nullptr;

    };

}

#endif //AUL_MULTISPAN_HPP

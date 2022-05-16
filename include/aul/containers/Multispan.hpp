//
// Created by avereniect on 1/11/22.
//

#ifndef AUL_MULTISPAN_HPP
#define AUL_MULTISPAN_HPP

#include "Zipperator.hpp"
#include "../Utility.hpp"

#include <cstdint>
#include <type_traits>
#include <iterator>
#include <array>

namespace aul {

    ///
    /// Constant which if passed to aul::Span<T, E> as E indicates that the
    /// span's extent isn't known at compile time, i.e. dynamic.
    ///
    static constexpr std::size_t dynamic_extent = -1;

    /*
    ///
    /// Base class for span classes.
    ///
    /// \tparam E Span extent
    template<std::size_t E>
    class Span_base {
    protected:

        Span_base() = default;

        explicit Span_base(std::size_t n) {}

        [[nodiscard]]
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

        [[nodiscard]]
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

        size_type size() const {
            return base::size();
        }

        size_type size_bytes() const {
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
            if (Count == dynamic_extent) {
                return {extent - Offset, ptr};
            } else {
                return {ptr + Offset};
            }
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
    */

    ///
    /// Multispan implementation class
    ///
    /// A span that functions as a view over parallel arrays. To facilitate the
    /// usage of this class as a dynamic-length mutlispan the aul::Multispan
    /// type alias exists and its usage should be preferred to the direct usage
    /// of this class template.
    ///
    /// TODO: Implement empty-base optimization in case where Extent is fixed
    ///
    /// \tparam Extent Length of multispan. Dynamic if equal to dynamic_extent
    /// \tparam Args Element types of arrays
    template<std::size_t Extent, class...Args>
    class Multispan_impl {
    public:

        //=================================================
        // Type aliases
        //=================================================

        using iterator = Random_access_zipperator<Args*...>;

        using reverse_iterator = std::reverse_iterator<iterator>;

        using element_type = std::tuple<Args...>;
        using value_type = std::tuple<std::remove_cv_t<Args>...>;

        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        using pointer = std::tuple<Args*...>;
        using const_pointer = std::tuple<const Args*...>;

        using reference = typename std::iterator_traits<iterator>::reference;
        using const_reference = std::tuple<std::reference_wrapper<std::add_const<Args>>...>;

        //=================================================
        // -ctors
        //=================================================

        template<class...Its>
        explicit Multispan_impl(Its...its):
            it(its...) {}

        //TODO: Add constructors from primitive arrays and std::array

        Multispan_impl() = default;
        Multispan_impl(const Multispan_impl&) = default;
        Multispan_impl(Multispan_impl&&) noexcept = default;
        ~Multispan_impl() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Multispan_impl& operator=(const Multispan_impl&) = default;
        Multispan_impl& operator=(Multispan_impl&&) noexcept = default;

        //=================================================
        // Iterator methods
        //=================================================

        iterator begin() const {
            return it;
        }

        iterator end() const {
            return it + size();
        }

        //=================================================
        // Element accessors
        //=================================================

        reference front() const {
            return it[0];
        }

        reference back() const {
            return it[size() - 1];
        }

        reference operator[](size_type idx) const {
            return it[idx];
        }

        pointer data() const {
            return it.operator->();
        }

        //=================================================
        // Accessors
        //=================================================

        [[nodiscard]]
        size_type size() const {
            return Extent;
        }

        [[nodiscard]]
        size_type size_bytes() const {
            return size() * aul::sizeof_sum<Args...>::value;
        }

        [[nodiscard]]
        bool empty() const {
            return size() == 0;
        }

        //=================================================
        // Subviews
        //=================================================

        template<std::size_t Count>
        Multispan_impl<Count, Args...> first() const {
            Multispan_impl<Count, Args...> ret;
            ret.it = it;
            return ret;
        }

        Multispan_impl<dynamic_extent, element_type> first(std::size_t c) const {
            return Multispan_impl<dynamic_extent, element_type>{it, c};
        }

        template<std::size_t Offset, std::size_t Count = dynamic_extent>
        Multispan_impl<(Count == dynamic_extent) ? dynamic_extent : (Extent != dynamic_extent) ? (Extent - Count) : dynamic_extent, element_type>
        subspan() const {
            if (Count == dynamic_extent) {
                return {Extent - Offset, it};
            } else {
                return {it + Offset};
            }
        }

        Multispan_impl<dynamic_extent, element_type> subspan(size_type offset, size_type Count = dynamic_extent) const {
            size_type new_size = (Count == dynamic_extent) ? (size() - offset) : Count;
            return {new_size, begin() + offset};
        }

        //=================================================
        // Conversion operators
        //=================================================

        operator Multispan_impl<Extent, const Args...>() const {
            return {it, size()};
        };

    private:

        //=================================================
        // Instance members
        //=================================================

        iterator it{};

    };

    ///
    /// Specialization of Multispan for case where it's a scalar span and thus
    /// basically a C++11 compatible std::span implementation.
    ///
    /// \tparam T
    template<class T>
    class Multispan_impl<dynamic_extent, T> {
    public:

        static constexpr std::size_t extent = dynamic_extent;

        //=================================================
        // Type aliases
        //=================================================

        using iterator = T*;
        using reverse_iterator = std::reverse_iterator<T*>;

        using element_type = T;
        using value_type = typename std::remove_cv<T>::type;

        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        using pointer = T*;
        using const_pointer = T const*;

        using reference = T&;
        using const_reference = const T&;

        //=================================================
        // -ctors
        //=================================================

        ///
        /// This overload is provided in order for scalar spans to have an
        /// interface closer to that of std::span.
        ///
        /// \tparam It Iterator type
        /// \param first Iterator to first element in
        /// \param count
        template<class It>
        Multispan_impl(It first, size_type count):
            elem_count(count),
            ptr(std::addressof(*first)) {}

        ///
        /// This overload is provided for consistency with spans of multiple
        /// ranges where the size parameter has to be specified first since the
        /// parameter pack has to come last (or at least, it would be much
        /// harder to have it come first to the point where it would not be
        /// worth the effort)
        ///
        /// \tparam It Iterator type
        /// \param first Iterator to first element in
        /// \param count
        template<class It>
        Multispan_impl(size_type count, It first):
            elem_count(count),
            ptr(std::addressof(*first)) {}

        ///
        /// \tparam It
        /// \tparam End
        /// \param first
        /// \param end
        template<class It, class End>
        Multispan_impl(It first, End end):
            elem_count(std::addressof(*end) - std::addressof(*first)),
            ptr(std::addressof(*first)) {}

        ///
        /// \tparam N Length of primitive array.
        /// \param arr Primitive array to span over
        template<std::size_t N>
        explicit Multispan_impl(element_type (&arr)[N]) noexcept:
            elem_count(N),
            ptr(arr) {}

        ///
        /// \tparam N Length of std::array
        /// \param arr std::array object to span over
        template<std::size_t N>
        explicit Multispan_impl(std::array<element_type, N>& arr) noexcept:
            elem_count(N),
            ptr(arr.data()) {}

        ///
        /// \tparam N
        /// \param arr
        template<std::size_t N>
        explicit Multispan_impl(const std::array<std::remove_const_t<element_type>, N>& arr) noexcept:
            elem_count(N),
            ptr(arr.data()) {}

        Multispan_impl() = default;
        Multispan_impl(const Multispan_impl&) = default;
        Multispan_impl(Multispan_impl&&) noexcept = default;
        ~Multispan_impl() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Multispan_impl& operator=(const Multispan_impl&) = default;
        Multispan_impl& operator=(Multispan_impl&&) noexcept = default;

        //=================================================
        // Iterator methods
        //=================================================

        iterator begin() const {
            return ptr;
        }

        iterator end() const {
            return {ptr + size()};
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

        [[nodiscard]]
        size_type size() const {
            return elem_count;
        }

        [[nodiscard]]
        size_type size_bytes() const {
            return size() * sizeof(T);
        }

        [[nodiscard]]
        bool empty() const {
            return size() == 0;
        }

        //=================================================
        // Subviews
        //=================================================

        template<std::size_t Count>
        Multispan_impl<Count, T> first() const {
            return {ptr};
        }

        Multispan_impl<dynamic_extent, element_type> first(std::size_t c) const {
            return Multispan_impl<dynamic_extent, element_type>{ptr, c};
        }

        template<std::size_t Offset, std::size_t Count = dynamic_extent>
        Multispan_impl<dynamic_extent, element_type> subspan() const {
            return {size() - Offset, ptr};
        }

        Multispan_impl<dynamic_extent, element_type> subspan(size_type offset, size_type Count = dynamic_extent) const {
            size_type new_size = (Count == dynamic_extent) ? (size() - offset) : Count;
            return {new_size, begin() + offset};
        }

        //=================================================
        // Conversion operators
        //=================================================

        operator Multispan_impl<dynamic_extent, const T>() const {
            return {ptr, size()};
        };

        //=================================================
        // Instance members
        //=================================================

    private:

        size_type elem_count = 0;

        T* ptr = nullptr;

    };

    template<class...Args>
    class Multispan_impl<dynamic_extent, Args...> {
    public:

        //=================================================
        // Type aliases
        //=================================================

        using iterator = Random_access_zipperator<Args*...>;

        using reverse_iterator = std::reverse_iterator<iterator>;

        using element_type = std::tuple<Args...>;
        using value_type = std::tuple<std::remove_cv_t<Args>...>;

        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        using pointer = std::tuple<Args*...>;
        using const_pointer = std::tuple<const Args*...>;;

        using reference = typename std::iterator_traits<iterator>::reference;
        using const_reference = typename std::iterator_traits<iterator>::reference;

        //=================================================
        // -ctors
        //=================================================

        template<class...Its>
        explicit Multispan_impl(size_type count, Its...its):
            elem_count(count),
            it(std::addressof(*its)...) {}

        //TODO: Add constructors from primitive arrays and std::array

        Multispan_impl() = default;
        Multispan_impl(const Multispan_impl&) = default;
        Multispan_impl(Multispan_impl&&) noexcept = default;
        ~Multispan_impl() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Multispan_impl& operator=(const Multispan_impl&) = default;
        Multispan_impl& operator=(Multispan_impl&&) noexcept = default;

        //=================================================
        // Iterator methods
        //=================================================

        iterator begin() const {
            return it;
        }

        iterator end() const {
            return {it + elem_count};
        }

        //=================================================
        // Element accessors
        //=================================================

        reference front() const {
            return it[0];
        }

        reference back() const {
            return it[elem_count - 1];
        }

        reference operator[](size_type idx) const {
            return it[idx];
        }

        pointer data() const {
            return it.operator->();
        }

        //=================================================
        // Accessors
        //=================================================

        [[nodiscard]]
        size_type size() const {
            return elem_count;
        }

        [[nodiscard]]
        size_type size_bytes() const {
            return size() * aul::sizeof_sum<Args...>::value;
        }

        [[nodiscard]]
        bool empty() const {
            return size() == 0;
        }

        //=================================================
        // Subviews
        //=================================================

        template<std::size_t Count>
        Multispan_impl<Count, Args...> first() const {
            Multispan_impl<Count, Args...> ret;
            ret.it = it;
            return ret;
        }

        Multispan_impl<dynamic_extent, element_type> first(std::size_t c) const {
            return Multispan_impl<dynamic_extent, element_type>{it, c};
        }

        template<std::size_t Offset, std::size_t Count = dynamic_extent>
        Multispan_impl<Count, element_type> subspan() const {
            if (Count == dynamic_extent) {
                return {size() - Offset, it};
            } else {
                return {it + Offset};
            }
        }

        Multispan_impl<dynamic_extent, element_type> subspan(size_type offset, size_type Count = dynamic_extent) const {
            size_type new_size = (Count == dynamic_extent) ? (size() - offset) : Count;
            return {new_size, begin() + offset};
        }

        //=================================================
        // Conversion operators
        //=================================================

        operator Multispan_impl<dynamic_extent, const Args...>() const {
            return {it, size()};
        };

    private:

        //=================================================
        // Instance members
        //=================================================

        size_type elem_count = 0;

        iterator it{};

    };

    template<std::size_t Extent, class T>
    class Multispan_impl<Extent, T>{
    public:

        static constexpr std::size_t extent = dynamic_extent;

        //=================================================
        // Type aliases
        //=================================================

        using iterator = T*;
        using reverse_iterator = std::reverse_iterator<T*>;

        using element_type = T;
        using value_type = typename std::remove_cv<T>::type;

        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        using pointer = T*;
        using const_pointer = T const*;

        using reference = T&;
        using const_reference = const T&;

        ///
        /// \tparam It
        /// \param first
        template<class It>
        explicit Multispan_impl(It first):
            ptr(std::addressof(*first)) {}

        ///
        /// \tparam N Length of primitive array.
        /// \param arr Primitive array to span over
        template<std::size_t N, class = typename std::enable_if_t<Extent <= N>>
        explicit Multispan_impl(element_type (&arr)[N]) noexcept:
            ptr(arr) {}

        ///
        /// \tparam N Length of std::array
        /// \param arr std::array object to span over
        template<std::size_t N, class = typename std::enable_if_t<Extent <= N>>
        explicit Multispan_impl(std::array<element_type, N>& arr) noexcept:
            ptr(arr) {}

        ///
        /// \tparam N
        /// \param arr
        template<std::size_t N, class = typename std::enable_if_t<Extent <= N>>
        explicit Multispan_impl(const std::array<std::remove_const_t<element_type>, N>& arr) noexcept:
            ptr(arr) {}

        Multispan_impl() = default;
        Multispan_impl(const Multispan_impl&) = default;
        Multispan_impl(Multispan_impl&&) noexcept = default;
        ~Multispan_impl() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Multispan_impl& operator=(const Multispan_impl&) = default;
        Multispan_impl& operator=(Multispan_impl&&) noexcept = default;

        //=================================================
        // Iterator methods
        //=================================================

        iterator begin() const {
            return iterator{ptr};
        }

        iterator end() const {
            return iterator{ptr + Extent};
        }

        //=================================================
        // Element accessors
        //=================================================

        reference front() const {
            return ptr[0];
        }

        reference back() const {
            return ptr[extent - 1];
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

        [[nodiscard]]
        size_type size() const {
            return extent;
        }

        [[nodiscard]]
        size_type size_bytes() const {
            return size() * sizeof(element_type);
        }

        [[nodiscard]]
        bool empty() const {
            return size() == 0;
        }

        //=================================================
        // Subviews
        //=================================================

        template<std::size_t Count>
        Multispan_impl<Count, element_type> first() const {
            Multispan_impl<Count, element_type> ret;
            ret.ptr = ptr;
            return ret;
        }

        Multispan_impl<dynamic_extent, element_type> first(std::size_t c) const {
            return {ptr, c};
        }

        template<std::size_t Offset, std::size_t Count = dynamic_extent>
        [[nodiscard]]
        Multispan_impl<(Count == dynamic_extent) ? dynamic_extent : (Extent - Count), element_type>
        subspan() const {
            return {ptr + Offset};
        }

        Multispan_impl<dynamic_extent, element_type> subspan(size_type offset, size_type Count = dynamic_extent) const {
            size_type new_size = (Count == dynamic_extent) ? (size() - offset) : Count;
            return {new_size, begin() + offset};
        }

        //=================================================
        // Conversion operators
        //=================================================

        operator Multispan_impl<Extent, const T>() const {
            return {ptr, size()};
        };

    private:

        //=================================================
        // Instance members
        //=================================================

        pointer ptr{};

    };

    ///
    /// Convenience type alias meant to ease the use of scalar spans.
    ///
    template<class T, std::size_t Extent = dynamic_extent>
    using Span = Multispan_impl<Extent, T>;

    ///
    ///
    ///
    template<std::size_t Extent, class...Args>
    using Fixed_multispan = Multispan_impl<Extent, Args...>;

    ///
    /// Type aliases that allows for use of a dynamic-length multispan without
    /// the need to explicitly specify the Extent template parameter.
    ///
    template<class...Args>
    using Multispan = Multispan_impl<dynamic_extent, Args...>;

}

#endif //AUL_MULTISPAN_HPP

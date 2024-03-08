#ifndef AUL_SPAN_HPP
#define AUL_SPAN_HPP

#include "aul/containers/Zipper_iterator.hpp"
#include "Utility.hpp"

#include <cstdint>
#include <type_traits>
#include <iterator>
#include <array>

namespace aul {

    //=====================================================
    // Static constants
    //=====================================================

    ///
    /// Constant which if passed to aul::Span<T, E> as E indicates that the
    /// span's extent isn't known at compile time, i.e. dynamic.
    ///
    static constexpr std::size_t dynamic_extent = SIZE_MAX;

    //=====================================================
    // Multi_span class implementations
    //=====================================================

    ///
    /// Multispan implementation base class
    ///
    /// A span that functions as a view over parallel arrays. To facilitate the
    /// usage of this class as a dynamic-length mutlispan the aul::Multispan
    /// type alias exists and its usage should be preferred to the direct usage
    /// of this class template.
    ///
    /// \tparam Extent Length of multispan. Dynamic if equal to dynamic_extent
    /// \tparam Args Element types of arrays
    template<std::size_t Extent, class...Args>
    class Multispan_impl {
    public:

        //=================================================
        // Type aliases
        //=================================================

        using iterator = Random_access_zipper_iterator<Args*...>;

        using reverse_iterator = std::reverse_iterator<iterator>;

        using element_type = std::tuple<Args...>;
        using value_type = std::tuple<typename std::remove_cv<Args>::type...>;

        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        using pointer = std::tuple<Args*...>;
        using const_pointer = std::tuple<const Args*...>;

        using reference = typename std::iterator_traits<iterator>::reference;
        using const_reference = std::tuple<std::reference_wrapper<std::add_const<Args>>...>;

        //=================================================
        // -ctors
        //=================================================

        ///
        /// \tparam Its List of contiguous iterator types
        /// \param its Iterators to beginning of ranges ot span over
        template<class...Its>
        explicit Multispan_impl(Its...its):
            it(std::addressof(*its)...) {}

        ///
        /// \tparam N Length of primitive array. Should be less than or equal to
        /// Extent
        /// \param args List of primitive arrays
        template<std::size_t N, class = typename std::enable_if<Extent <= N>::type>
        explicit Multispan_impl(Args (&...args)[N]):
            it(args...) {}

        ///
        /// \tparam N Length of std::array. Should be less then or equal to
        /// Extent.
        /// \param args List of std::array objects
        template<std::size_t N, class = typename std::enable_if<Extent <= N>::type>
        explicit Multispan_impl(const std::array<typename std::remove_const<Args>::type, N> ...args):
            it(args.data()...) {}

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

        ///
        ///
        /// \return Iterator to beginning of range
        [[nodiscard]]
        iterator begin() const {
            return it;
        }

        ///
        ///
        /// \return Iterator to end of range
        [[nodiscard]]
        iterator end() const {
            return it + size();
        }

        //=================================================
        // Element accessors
        //=================================================

        ///
        /// Behavior is undefined if empty() is true
        ///
        /// \return References to the first elements in the span
        [[nodiscard]]
        reference front() const {
            return it[0];
        }

        ///
        /// Behavior is undefined if empty() is true
        ///
        /// \return References to the last elements in the span
        [[nodiscard]]
        reference back() const {
            return it[size() - 1];
        }

        ///
        /// \param idx Index of elements to retrieve a reference to
        /// \return References to the idx'th elements in the span
        [[nodiscard]]
        reference operator[](size_type idx) const {
            return it[idx];
        }

        ///
        /// \return Pointers to first elements in the span
        [[nodiscard]]
        pointer data() const {
            return it.operator->();
        }

        //=================================================
        // Accessors
        //=================================================

        ///
        /// \return The number of elements over which the multispan acts as a view
        [[nodiscard]]
        size_type size() const {
            return Extent;
        }

        ///
        /// \return The number of bytes in the objects over which the multispan
        /// acts as a view
        [[nodiscard]]
        size_type size_bytes() const {
            return size() * aul::sizeof_sum<Args...>::value;
        }

        ///
        /// \return True if the extent of the span is 0 elements.
        [[nodiscard]]
        bool empty() const {
            return size() == 0;
        }

        //=================================================
        // Subviews
        //=================================================

        ///
        /// \tparam Count The number of elements for the new multispan to act as
        /// a view over. Should be not greater than the value returned by size()
        /// \return A new multispan that acts as a view over the first Count
        /// elements in the current multispan
        template<std::size_t Count>
        [[nodiscard]]
        Multispan_impl<Count, Args...> first() const {
            Multispan_impl<Count, Args...> ret;
            ret.it = it;
            return ret;
        }

        ///
        /// \param c The number of elements for the new mutispan to act as a
        /// view over. Should be not greater than the value returned by size()
        /// \return A new multispan that acts as a view over the first c
        /// elements in the current multispan
        [[nodiscard]]
        Multispan_impl<dynamic_extent, element_type> first(std::size_t c) const {
            return Multispan_impl<dynamic_extent, element_type>{it, c};
        }

        ///
        /// \tparam Offset Starting index from which new multispan should begin
        /// \tparam Count Number of elements in new multispan
        /// \return A new multispan which acts as a view over a subset of the
        /// current multispan
        template<std::size_t Offset, std::size_t Count = dynamic_extent>
        [[nodiscard]]
        Multispan_impl<(Count == dynamic_extent) ? dynamic_extent : (Extent != dynamic_extent) ? (Extent - Count) : dynamic_extent, element_type>
        subspan() const {
            if (Count == dynamic_extent) {
                return {Extent - Offset, it};
            } else {
                return {it + Offset};
            }
        }

        ///
        /// \param offset Starting index from which new multispan should begin
        /// \param Count Number of elements in new multispan
        /// \return A new multispan which acts as a view over a subset of the
        /// current span
        [[nodiscard]]
        Multispan_impl<dynamic_extent, element_type> subspan(size_type offset, size_type Count = dynamic_extent) const {
            size_type new_size = (Count == dynamic_extent) ? (size() - offset) : Count;
            return {new_size, begin() + offset};
        }

        //=================================================
        // Conversion operators
        //=================================================

        ///
        /// Implicit conversion to multispan over const ranges so that
        /// multispans over non-const objects can easily be used where
        /// multispans over const objects are required
        ///
        [[nodiscard]]
        operator Multispan_impl<Extent, const Args...>() const {
            return {it, size()};
        };

        ///
        /// Implicit conversion to span of dynamic extent.
        ///
        [[nodiscard]]
        operator Multispan_impl<dynamic_extent, Args...>() const {
            return {it, size()};
        };

        ///
        /// Implicit conversion to span of dynamic extent over const range.
        ///
        [[nodiscard]]
        operator Multispan_impl<dynamic_extent, const Args...>() const {
            return {it, size()};
        };

    private:

        //=================================================
        // Instance members
        //=================================================

        iterator it{};

    };


    ///
    /// A class that acts as a view over a range of contiguously allocated
    /// elements or set thereof.
    ///
    /// Specialization for span with a dynamic extent.
    ///
    /// \tparam T The type of the object over which the span should act as a
    /// view
    template<class...Args>
    class Multispan_impl<dynamic_extent, Args...> {
    public:

        //=================================================
        // Type aliases
        //=================================================

        using iterator = Random_access_zipper_iterator<Args*...>;

        using reverse_iterator = std::reverse_iterator<iterator>;

        using element_type = std::tuple<Args...>;
        using value_type = std::tuple<typename std::remove_cv<Args>::type...>;

        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        using pointer = std::tuple<Args*...>;
        using const_pointer = std::tuple<const Args*...>;;

        using reference = typename std::iterator_traits<iterator>::reference;
        using const_reference = typename std::iterator_traits<iterator>::reference;

        //=================================================
        // -ctors
        //=================================================

        ///
        /// \tparam Its List of contiguous iterator types
        /// \param count Number of elements in ranges
        /// \param its Iterators to beginning of ranges ot span over
        template<class...Its>
        explicit Multispan_impl(size_type count, Its...its):
            elem_count(count),
            it(std::addressof(*its)...) {}

        ///
        /// \tparam N Length of primitive array.
        /// \param args List of primitive arrays
        template<std::size_t N>
        explicit Multispan_impl(Args (&...args)[N]):
            elem_count(N),
            it(args...) {}

        ///
        /// \tparam N Length of std::array objects
        /// \param args List of std::array objects
        template<std::size_t N>
        explicit Multispan_impl(const std::array<typename std::remove_const<Args>::type, N> ...args):
            elem_count(N),
            it(args.data()...) {}

        ///
        /// \tparam N Width of fixed-width span
        /// \param other Fixed-width span over same element types
        template<std::size_t N>
        Multispan_impl(Multispan_impl<N, Args...> other):
            elem_count(N),
            it(other.data()) {}

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

        ///
        ///
        /// \return Iterator to beginning of range
        [[nodiscard]]
        iterator begin() const {
            return it;
        }

        ///
        ///
        /// \return Iterator to end of range
        [[nodiscard]]
        iterator end() const {
            return {it + elem_count};
        }

        //=================================================
        // Element accessors
        //=================================================

        ///
        /// Behavior is undefined if empty() is true
        ///
        /// \return References to the first elements in the span
        [[nodiscard]]
        reference front() const {
            return it[0];
        }

        ///
        /// Behavior is undefined if empty() is true
        ///
        /// \return References to the last elements in the span
        [[nodiscard]]
        reference back() const {
            return it[elem_count - 1];
        }

        ///
        /// \param idx Index of elements to retrieve a reference to
        /// \return References to the idx'th elements in the span
        [[nodiscard]]
        reference operator[](size_type idx) const {
            return it[idx];
        }

        ///
        /// \return Pointers to first elements in the span
        [[nodiscard]]
        pointer data() const {
            return it.operator->();
        }

        //=================================================
        // Accessors
        //=================================================

        ///
        /// \return The number of elements over which the multispan acts as a view
        [[nodiscard]]
        size_type size() const {
            return elem_count;
        }

        ///
        /// \return The number of bytes in the objects over which the multispan
        /// acts as a view
        [[nodiscard]]
        size_type size_bytes() const {
            return size() * aul::sizeof_sum<Args...>::value;
        }

        ///
        /// \return True if the extent of the span is 0 elements.
        [[nodiscard]]
        bool empty() const {
            return size() == 0;
        }

        //=================================================
        // Subviews
        //=================================================

        ///
        /// \tparam Count The number of elements for the new multispan to act as
        /// a view over. Should be not greater than the value returned by size()
        /// \return A new multispan that acts as a view over the first Count
        /// elements in the current multispan
        template<std::size_t Count>
        [[nodiscard]]
        Multispan_impl<Count, Args...> first() const {
            Multispan_impl<Count, Args...> ret;
            ret.it = it;
            return ret;
        }

        ///
        /// \param c The number of elements for the new mutispan to act as a
        /// view over. Should be not greater than the value returned by size()
        /// \return A new multispan that acts as a view over the first c
        /// elements in the current multispan
        [[nodiscard]]
        Multispan_impl<dynamic_extent, element_type> first(std::size_t c) const {
            return Multispan_impl<dynamic_extent, element_type>{it, c};
        }

        ///
        /// \tparam Offset Starting index from which new multispan should begin
        /// \tparam Count Number of elements in new multispan
        /// \return A new multispan which acts as a view over a subset of the
        /// current multispan
        template<std::size_t Offset, std::size_t Count = dynamic_extent>
        [[nodiscard]]
        Multispan_impl<Count, element_type> subspan() const {
            if (Count == dynamic_extent) {
                return {size() - Offset, it};
            } else {
                return {it + Offset};
            }
        }

        ///
        /// \param offset Starting index from which new multispan should begin
        /// \param Count Number of elements in new multispan
        /// \return A new multispan which acts as a view over a subset of the
        /// current span
        [[nodiscard]]
        Multispan_impl<dynamic_extent, element_type> subspan(size_type offset, size_type Count = dynamic_extent) const {
            size_type new_size = (Count == dynamic_extent) ? (size() - offset) : Count;
            return {new_size, begin() + offset};
        }

        //=================================================
        // Conversion operators
        //=================================================

        ///
        /// Implicit conversion to multispan over const ranges so that
        /// multispans over non-const objects can easily be used where
        /// multispans over const objects are required
        ///
        [[nodiscard]]
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


    ///
    /// A class that acts as a view over a range of contiguously allocated
    /// elements
    ///
    /// Base case for multispans.
    ///
    /// \tparam Extent The number of elements over which the span should act as
    /// a view. If Extent is equal to dynamic_extent, then the span's extent is
    /// a run-time value.
    /// \tparam T The type of the objects over which the span should act as a
    /// view
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
        /// \tparam It Contiguous iterator type
        /// \param first Iterator to beginning of range to span over
        template<class It>
        [[nodiscard]]
        explicit Multispan_impl(It first):
            ptr(std::addressof(*first)) {}

        ///
        /// \tparam N Length of primitive array. Should be less than or equal to
        /// Extent
        /// \param arr Primitive array to span over
        template<std::size_t N, class = typename std::enable_if<Extent <= N>::type>
        [[nodiscard]]
        explicit Multispan_impl(element_type (&arr)[N]) noexcept:
            ptr(arr) {}

        ///
        /// \tparam N Length of std::array object. Should be less than or equal
        /// to Extent
        /// \param arr std::array object to span over
        template<std::size_t N, class = typename std::enable_if<Extent <= N>::type>
        [[nodiscard]]
        explicit Multispan_impl(std::array<element_type, N>& arr) noexcept:
            ptr(arr) {}

        ///
        /// \tparam N Length of std::array object. Should be less than Extent
        /// \param arr std::array object to span over
        template<std::size_t N, class = typename std::enable_if<Extent <= N>::type>
        [[nodiscard]]
        explicit Multispan_impl(const std::array<typename std::remove_const<element_type>::type, N>& arr) noexcept:
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

        ///
        ///
        /// \return Iterator to beginning of range
        [[nodiscard]]
        iterator begin() const {
            return iterator{ptr};
        }

        ///
        ///
        /// \return Iterator to end of range
        [[nodiscard]]
        iterator end() const {
            return iterator{ptr + Extent};
        }

        //=================================================
        // Element accessors
        //=================================================

        ///
        /// Behavior is undefined if empty() is true
        ///
        /// \return Reference to the first element in the span
        [[nodiscard]]
        reference front() const {
            return ptr[0];
        }

        ///
        /// Behavior is undefined if empty() is true
        ///
        /// \return Reference to the last element in the span
        [[nodiscard]]
        reference back() const {
            return ptr[extent - 1];
        }

        ///
        /// \param idx Index of element to retrieve a reference to
        /// \return Reference to the idx'th element in the span
        [[nodiscard]]
        reference operator[](size_type idx) const {
            return ptr[idx];
        }

        ///
        /// \return Pointer to first element in the span
        [[nodiscard]]
        pointer data() const {
            return ptr;
        }

        //=================================================
        // Accessors
        //=================================================

        ///
        /// \return The number of elements over which the span acts as a view
        [[nodiscard]]
        size_type size() const {
            return extent;
        }

        ///
        /// \return The number of bytes in the objects over which the span acts
        /// as a view
        [[nodiscard]]
        size_type size_bytes() const {
            return size() * sizeof(element_type);
        }

        ///
        /// \return True if the extent of the span is 0 elements.
        [[nodiscard]]
        bool empty() const {
            return size() == 0;
        }

        //=================================================
        // Subviews
        //=================================================

        ///
        /// \tparam Count The number of elements for the new span to act as a
        /// view over. Should be not greater than the value returned by size()
        /// \return A new span that acts as a view over the first Count elements
        /// in the current span
        template<std::size_t Count>
        [[nodiscard]]
        Multispan_impl<Count, element_type> first() const {
            Multispan_impl<Count, element_type> ret;
            ret.ptr = ptr;
            return ret;
        }

        ///
        /// \param c The number of elements for the new span to act as a view
        /// over. Should be not greater than the value returned by size()
        /// \return A new span that acts as a view over the first c elements in
        /// the current span
        [[nodiscard]]
        Multispan_impl<dynamic_extent, element_type> first(std::size_t c) const {
            return {ptr, c};
        }

        ///
        /// \tparam Offset Starting index from which new span should begin
        /// \tparam Count Number of elements in new span
        /// \return A new span which acts as a view over a subset of the current
        /// span
        template<std::size_t Offset, std::size_t Count = dynamic_extent>
        [[nodiscard]]
        Multispan_impl<(Count == dynamic_extent) ? dynamic_extent : (Extent - Count), element_type>
        subspan() const {
            return {ptr + Offset};
        }

        ///
        /// \param offset Starting index from which new span should begin
        /// \param Count Number of elements in new span
        /// \return A new span which acts as a view over a subset of the current
        /// span
        [[nodiscard]]
        Multispan_impl<dynamic_extent, element_type> subspan(size_type offset, size_type Count = dynamic_extent) const {
            size_type new_size = (Count == dynamic_extent) ? (size() - offset) : Count;
            return {new_size, begin() + offset};
        }

        //=================================================
        // Conversion operators
        //=================================================

        ///
        /// Implicit conversion to span over const range so that spans over
        /// non-const objects can easily be used where spans over const objects
        /// are required
        ///
        [[nodiscard]]
        operator Multispan_impl<Extent, const T>() const {
            return {ptr, size()};
        };

        ///
        /// Implicit conversion to span of dynamic extent.
        ///
        [[nodiscard]]
        operator Multispan_impl<dynamic_extent, T>() const {
            return {ptr, size()};
        };

        /* TODO: Use SFINAE to conditionally enable this conversion
        ///
        /// Implicit conversion to span of dynamic extent over const range.
        ///
        [[nodiscard]]
        operator Multispan_impl<dynamic_extent, const T>() const {
            return {ptr, size()};
        };
        */

    private:

        //=================================================
        // Instance members
        //=================================================

        pointer ptr{};

    };


    ///
    /// Specialization of Multispan for case where it's a scalar span and thus
    /// basically a C++11 compatible std::span implementation.
    ///
    /// Base case for multispans with a dynamic extent.
    ///
    /// \tparam T The type of objects over which the span should act as a view
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
        ///
        /// \tparam It Contiguous iterator type
        /// \tparam End Contiguous iterator type
        /// \param first Iterator to beginning of range to span over
        /// \param end Iterator to end of range to span over
        template<
            class It,
            class End,
            class = typename std::enable_if<
                aul::is_dereferenceable<It>::value &&
                aul::is_dereferenceable<End>::value
            >::type
        >
        Multispan_impl(It first, End end):
            elem_count(std::addressof(*end) - std::addressof(*first)),
            ptr(std::addressof(*first)) {}

        ///
        /// \tparam N Length of primitive array. Should be less than or equal to
        /// Extent
        /// \param arr Primitive array to span over
        template<std::size_t N>
        explicit Multispan_impl(element_type (&arr)[N]) noexcept:
            elem_count(N),
            ptr(arr) {}

        ///
        /// \tparam N Length of std::array object. Should be less than or equal
        /// to Extent
        /// \param arr std::array object to span over
        template<std::size_t N>
        explicit Multispan_impl(std::array<element_type, N>& arr) noexcept:
            elem_count(N),
            ptr(arr.data()) {}

        ///
        /// \tparam N Length of std::array object. Should be less than Extent
        /// \param arr std::array object to span over
        template<std::size_t N>
        explicit Multispan_impl(const std::array<typename std::remove_const<element_type>::type, N>& arr) noexcept:
            elem_count(N),
            ptr(arr.data()) {}

        ///
        /// \tparam N Number of elements in fixed-width span
        /// \param other A fixed-width span with the same element type
        template<std::size_t N>
        Multispan_impl(Multispan_impl<N, T> other) noexcept:
            elem_count(N),
            ptr(other.data()) {}

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

        ///
        ///
        /// \return Iterator to beginning of range
        [[nodiscard]]
        iterator begin() const {
            return ptr;
        }

        ///
        ///
        /// \return Iterator to end of range
        [[nodiscard]]
        iterator end() const {
            return {ptr + size()};
        }

        //=================================================
        // Element accessors
        //=================================================

        ///
        /// Behavior is undefined if empty() is true
        ///
        /// \return Reference to the first element in the span
        [[nodiscard]]
        reference front() const {
            return ptr[0];
        }

        ///
        /// Behavior is undefined if empty() is true
        ///
        /// \return Reference to the last element in the span
        [[nodiscard]]
        reference back() const {
            return ptr[size() - 1];
        }

        ///
        /// \param idx Index of element to retrieve a reference to
        /// \return Reference to the idx'th element in the span
        [[nodiscard]]
        reference operator[](size_type idx) const {
            return ptr[idx];
        }

        ///
        /// \return Pointer to first element in the span
        [[nodiscard]]
        pointer data() const {
            return ptr;
        }

        //=================================================
        // Accessors
        //=================================================

        ///
        /// \return The number of elements over which the span acts as a view
        [[nodiscard]]
        size_type size() const {
            return elem_count;
        }

        ///
        /// \return The number of bytes in the objects over which the span acts
        /// as a view
        [[nodiscard]]
        size_type size_bytes() const {
            return size() * sizeof(T);
        }

        ///
        /// \return True if the extent of the span is 0 elements.
        [[nodiscard]]
        bool empty() const {
            return size() == 0;
        }

        //=================================================
        // Subviews
        //=================================================

        ///
        /// \tparam Count The number of elements for the new span to act as a
        /// view over. Should be not greater than the value returned by size()
        /// \return A new span that acts as a view over the first Count elements
        /// in the current span
        template<std::size_t Count>
        [[nodiscard]]
        Multispan_impl<Count, T> first() const {
            return {ptr};
        }

        ///
        /// \param c The number of elements for the new span to act as a view
        /// over. Should be not greater than the value returned by size()
        /// \return A new span that acts as a view over the first c elements in
        /// the current span
        [[nodiscard]]
        Multispan_impl<dynamic_extent, element_type> first(std::size_t c) const {
            return Multispan_impl<dynamic_extent, element_type>{ptr, c};
        }

        ///
        /// \tparam Offset Starting index from which new span should begin
        /// \tparam Count Number of elements in new span
        /// \return A new span which acts as a view over a subset of the current
        /// span
        template<std::size_t Offset, std::size_t Count = dynamic_extent>
        [[nodiscard]]
        Multispan_impl<dynamic_extent, element_type> subspan() const {
            return {size() - Offset, ptr};
        }

        ///
        /// \param offset Starting index from which new span should begin
        /// \param Count Number of elements in new span
        /// \return A new span which acts as a view over a subset of the current
        /// span
        [[nodiscard]]
        Multispan_impl<dynamic_extent, element_type> subspan(size_type offset, size_type Count = dynamic_extent) const {
            size_type new_size = (Count == dynamic_extent) ? (size() - offset) : Count;
            return {new_size, begin() + offset};
        }

        //=================================================
        // Conversion operators
        //=================================================

        ///
        /// Implicit conversion to span over const range so that spans over
        /// non-const objects can easily be used where spans over const objects
        /// are required
        ///
        [[nodiscard]]
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

    //=====================================================
    // Convenience interface
    //=====================================================

    ///
    /// A class which provides a non-owning view over a range of contiguously
    /// allocated objects.
    ///
    /// \tparam T Type of objects to views
    /// \tparam Extent Length of view. Length is dynamic when Extent is
    /// aul::dynamic_extent
    template<class T, std::size_t Extent = dynamic_extent>
    class Span : public Multispan_impl<Extent, T> {

        using Multispan_impl<Extent, T>::Multispan_impl;

    };

    ///
    /// A class which provides a non-owning view over a range of contiguously
    /// allocated objects of a specified length.
    ///
    /// \tparam Extent Length of view. Length is dynamic when Extent is
    // aul::dynamic_extent
    /// \tparam Args Types of objects in ranges to view.
    template<std::size_t Extent, class...Args>
    class Fixed_multispan : public Multispan_impl<Extent, Args...> {

        using Multispan_impl<Extent, Args...>::Multispan_impl;

    };

    ///
    /// A class which provides a non-owning view over multiple ranges of
    /// contiguously allocated objects, all of the same length.
    ///
    /// \tparam Args Types of objects in ranges to view.
    template<class...Args>
    class Multispan : public Multispan_impl<dynamic_extent, Args...> {

        using Multispan_impl<dynamic_extent, Args...>::Multispan_impl;

    };

    //=====================================================
    // Tuple methods
    //=====================================================

    template<std::size_t N, std::size_t Extent, class...Args>
    aul::Span<typename nth_type<N, Args...>::type, Extent>
    get(const aul::Multispan_impl<Extent, Args...>& multi_span) {
        using std::get;

        auto begin = get<N>(multi_span.begin());
        auto end   = get<N>(multi_span.end());
        return aul::Span<typename nth_type<N, Args...>::type, Extent>{begin, end};
    }

    //=====================================================
    // Deduction guides
    //=====================================================

    #if __cplusplus >= 201703L

    template<class It>
    Span(It, std::size_t) -> Span<typename std::iterator_traits<It>::value_type, dynamic_extent>;

    template<class It, class End>
    Span(It, End) -> Span<typename std::iterator_traits<It>::value_type, dynamic_extent>;

    template<class T, std::size_t N>
    Span(T (&arr)[N]) -> Span<T, N>;

    template<class T, std::size_t N>
    Span(const std::array<T, N>& arr) -> Span<T, N>;



    template<class...Its>
    Multispan(std::size_t, Its...) -> Multispan<Its...>;

    template<std::size_t N, class...Args>
    Multispan(Args (&...args)[N]) -> Multispan<Args...>;

    template<std::size_t N, class...Args>
    Multispan(const std::array<Args, N>...) -> Multispan<Args...>;

    #endif

}

#endif //AUL_SPAN_HPP

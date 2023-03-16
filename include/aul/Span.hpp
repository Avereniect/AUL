#ifndef AUL_SPAN_HPP
#define AUL_SPAN_HPP

#include "aul/containers/Zipper_iterator.hpp"
#include "Utility.hpp"

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

        using iterator = Random_access_zipper_iterator<Args*...>;

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

        ///
        /// \tparam Its List of contiguous iterator types
        /// \param its Iterators to beginning of ranges ot span over
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

        ///
        ///
        /// \return Iterator to beginning of range
        iterator begin() const {
            return it;
        }

        ///
        ///
        /// \return Iterator to end of range
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
        reference front() const {
            return it[0];
        }

        ///
        /// Behavior is undefined if empty() is true
        ///
        /// \return References to the last elements in the span
        reference back() const {
            return it[size() - 1];
        }

        ///
        /// \param idx Index of elements to retrieve a reference to
        /// \return References to the idx'th elements in the span
        reference operator[](size_type idx) const {
            return it[idx];
        }

        ///
        /// \return Pointers to first elements in the span
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
        Multispan_impl<dynamic_extent, element_type> first(std::size_t c) const {
            return Multispan_impl<dynamic_extent, element_type>{it, c};
        }

        ///
        /// \tparam Offset Starting index from which new multispan should begin
        /// \tparam Count Number of elements in new multispan
        /// \return A new multispan which acts as a view over a subset of the
        /// current multispan
        template<std::size_t Offset, std::size_t Count = dynamic_extent>
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
        /// \tparam N Length of primitive array. Should be less than Extent
        /// \param arr Primitive array to span over
        template<std::size_t N>
        explicit Multispan_impl(element_type (&arr)[N]) noexcept:
            elem_count(N),
            ptr(arr) {}

        ///
        /// \tparam N Length of std::array object. Should be less than Extent
        /// \param arr std::array object to span over
        template<std::size_t N>
        explicit Multispan_impl(std::array<element_type, N>& arr) noexcept:
            elem_count(N),
            ptr(arr.data()) {}

        ///
        /// \tparam N Length of std::array object. Should be less than Extent
        /// \param arr std::array object to span over
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

        ///
        ///
        /// \return Iterator to beginning of range
        iterator begin() const {
            return ptr;
        }

        ///
        ///
        /// \return Iterator to end of range
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
        reference front() const {
            return ptr[0];
        }

        ///
        /// Behavior is undefined if empty() is true
        ///
        /// \return Reference to the last element in the span
        reference back() const {
            return ptr[size() - 1];
        }

        ///
        /// \param idx Index of element to retrieve a reference to
        /// \return Reference to the idx'th element in the span
        reference operator[](size_type idx) const {
            return ptr[idx];
        }

        ///
        /// \return Pointer to first element in the span
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
        Multispan_impl<Count, T> first() const {
            return {ptr};
        }

        ///
        /// \param c The number of elements for the new span to act as a view
        /// over. Should be not greater than the value returned by size()
        /// \return A new span that acts as a view over the first c elements in
        /// the current span
        Multispan_impl<dynamic_extent, element_type> first(std::size_t c) const {
            return Multispan_impl<dynamic_extent, element_type>{ptr, c};
        }

        ///
        /// \tparam Offset Starting index from which new span should begin
        /// \tparam Count Number of elements in new span
        /// \return A new span which acts as a view over a subset of the current
        /// span
        template<std::size_t Offset, std::size_t Count = dynamic_extent>
        Multispan_impl<dynamic_extent, element_type> subspan() const {
            return {size() - Offset, ptr};
        }

        ///
        /// \param offset Starting index from which new span should begin
        /// \param Count Number of elements in new span
        /// \return A new span which acts as a view over a subset of the current
        /// span
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


    ///
    /// A class that acts as a view over a range of contiguously allocated
    /// elements or set thereof.
    ///
    /// Specialization for
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

        ///
        /// \tparam Its List of contiguous iterator types
        /// \param count Number of elements in ranges
        /// \param its Iterators to beginning of ranges ot span over
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

        ///
        ///
        /// \return Iterator to beginning of range
        iterator begin() const {
            return it;
        }

        ///
        ///
        /// \return Iterator to end of range
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
        reference front() const {
            return it[0];
        }

        ///
        /// Behavior is undefined if empty() is true
        ///
        /// \return References to the last elements in the span
        reference back() const {
            return it[elem_count - 1];
        }

        ///
        /// \param idx Index of elements to retrieve a reference to
        /// \return References to the idx'th elements in the span
        reference operator[](size_type idx) const {
            return it[idx];
        }

        ///
        /// \return Pointers to first elements in the span
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
        Multispan_impl<dynamic_extent, element_type> first(std::size_t c) const {
            return Multispan_impl<dynamic_extent, element_type>{it, c};
        }

        ///
        /// \tparam Offset Starting index from which new multispan should begin
        /// \tparam Count Number of elements in new multispan
        /// \return A new multispan which acts as a view over a subset of the
        /// current multispan
        template<std::size_t Offset, std::size_t Count = dynamic_extent>
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
    /// \tparam T The type of the object over which the span should act as a
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
        explicit Multispan_impl(It first):
            ptr(std::addressof(*first)) {}

        ///
        /// \tparam N Length of primitive array. Should be less than Extent
        /// \param arr Primitive array to span over
        template<std::size_t N, class = typename std::enable_if_t<Extent <= N>>
        explicit Multispan_impl(element_type (&arr)[N]) noexcept:
            ptr(arr) {}

        ///
        /// \tparam N Length of std::array object. Should be less than Extent
        /// \param arr std::array object to span over
        template<std::size_t N, class = typename std::enable_if_t<Extent <= N>>
        explicit Multispan_impl(std::array<element_type, N>& arr) noexcept:
            ptr(arr) {}

        ///
        /// \tparam N Length of std::array object. Should be less than Extent
        /// \param arr std::array object to span over
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

        ///
        ///
        /// \return Iterator to beginning of range
        iterator begin() const {
            return iterator{ptr};
        }

        ///
        ///
        /// \return Iterator to end of range
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
        reference front() const {
            return ptr[0];
        }

        ///
        /// Behavior is undefined if empty() is true
        ///
        /// \return Reference to the last element in the span
        reference back() const {
            return ptr[extent - 1];
        }

        ///
        /// \param idx Index of element to retrieve a reference to
        /// \return Reference to the idx'th element in the span
        reference operator[](size_type idx) const {
            return ptr[idx];
        }

        ///
        /// \return Pointer to first element in the span
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

    private:

        //=================================================
        // Instance members
        //=================================================

        pointer ptr{};

    };

    ///
    /// Type alias that allows for use of a Span over a single range in a more
    /// user-friendly fashion.
    ///
    template<class T, std::size_t Extent = dynamic_extent>
    using Span = Multispan_impl<Extent, T>;

    ///
    /// Type alias that allows for use of a fixed-length multispan with a more
    /// user-friendly name
    ///
    template<std::size_t Extent, class...Args>
    using Fixed_multispan = Multispan_impl<Extent, Args...>;

    ///
    /// Type alias that allows for use of a dynamic-length multispan without
    /// the need to explicitly specify the Extent template parameter.
    ///
    template<class...Args>
    using Multispan = Multispan_impl<dynamic_extent, Args...>;

}

#endif //AUL_SPAN_HPP

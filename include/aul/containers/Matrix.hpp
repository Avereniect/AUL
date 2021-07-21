#ifndef AUL_MATRIX_HPP
#define AUL_MATRIX_HPP

#include "Random_access_iterator.hpp"
#include "../Utility.hpp"
#include "../memory/Memory.hpp"

#include <memory>
#include <algorithm>
#include <numeric>
#include <functional>
#include <cstdint>
#include <stdexcept>

namespace aul {

    ///
    /// \tparam T Type of matrix elements
    /// \tparam N Number of dimensions to matrix
    /// \tparam A Allocator type to get aliases from
    template<class T, std::size_t N, class A = std::allocator<T>, bool is_const = false>
    class Matrix_view {
    public:

        static_assert(N > 0);

        //=================================================
        // Type aliases
        //=================================================

        using value_type = T;

        using reference = T&;
        using const_reference = const T&;

        using pointer = typename std::allocator_traits<A>::pointer;
        using const_pointer = typename std::allocator_traits<A>::const_pointer;

        using size_type = typename std::allocator_traits<A>::size_type;
        using difference_type = typename std::allocator_traits<A>::difference_type;

        using iterator = aul::Random_access_iterator<A, false>;
        using const_iterator = aul::Random_access_iterator<A, true>;

        using dimension_type = std::array<size_type, N>;

    private:

        using lower_dimensional_view = Matrix_view<T, N - 1, A, is_const>;
        using const_lower_dimensional_ivew = Matrix_view<T, N - 1, A, true>;

        using const_size_type_ptr = const typename std::pointer_traits<const_pointer>::template rebind<size_type>;

        using Ptr = std::conditional_t<is_const, const_pointer, pointer>;
        using Ref = std::conditional_t<is_const, const_reference, reference>;

        using subscript_return_type = std::conditional_t<
            N == 1,
            Ref,
            lower_dimensional_view
        >;

        using const_subscript_return_type = std::conditional_t<
            N == 1,
            const_reference,
            const_lower_dimensional_ivew
        >;

    public:

        //=================================================
        // -ctors
        //=================================================

        Matrix_view() = default;

        explicit Matrix_view(Ptr ptr, const dimension_type& dims):
            ptr(ptr),
            dims(dims) {}

        explicit Matrix_view(Ptr ptr, const typename dimension_type::size_type* dim_ptr):
            ptr(ptr),
            dims() {

            std::copy_n(dim_ptr, N, dims.data());
        }

        Matrix_view(const Matrix_view&) = default;
        Matrix_view(Matrix_view&&) noexcept = default;
        ~Matrix_view() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Matrix_view& operator=(const Matrix_view&) = default;
        Matrix_view& operator=(Matrix_view&&) noexcept = default;

        //=================================================
        // Access methods
        //=================================================

        subscript_return_type operator[](const size_type n) {
            if constexpr (N == 1) {
                return ptr[n];
            } else {
                return lower_dimensional_view{ptr + compute_offset(n, dims), dims.data()};
            }
        }

        const_subscript_return_type operator[](const size_type n) const {
            if constexpr (N == 1) {
                return ptr[n];
            } else {
                return lower_dimensional_view{ptr + compute_offset(n, dims), dims.data()};
            }
        }

        template<class...Args, class = typename aul::enable_if_homogenous_N_t<N + 1, size_type, Args...>>
        Ref at(Args...args) {
            return at(aul::array_from_T<N, size_type>(args...));
        }

        Ref at(const std::array<size_type, N>& pos) {
            for (std::size_t i = 0; i < N; ++i) {
                if (dims[i] <= pos[i]) {
                    throw std::out_of_range("Index out of range in call to aul::Matrix_view::at().");
                }
            }

            size_type offset = 0;
            size_type coefficient = 1;
            for (std::size_t i = N; i-- > 0;) {
                offset += (coefficient * pos[i]);
                coefficient *= dims[i];
            }

            return ptr + offset;
        }

        template<class...Args, class = typename aul::enable_if_homogenous_N_t<N + 1, size_type, Args...>>
        const_reference at(Args...args) const {
            return at(aul::array_from_T<N, size_type>(args...));
        }

        const_reference at(const std::array<size_type, N>& pos) const {
            for (std::size_t i = 0; i < N; ++i) {
                if (dims[i] <= pos[i]) {
                    throw std::out_of_range("Index out of range in call to aul::Matrix_view::at().");
                }
            }

            size_type offset = 0;
            size_type coefficient = 1;
            for (std::size_t i = N; i-- > 0;) {
                offset += (coefficient * pos[i]);
                coefficient *= dims[i];
            }

            return ptr + offset;
        }

        //=================================================
        // Size methods
        //=================================================

        [[nodiscard]]
        dimension_type dimensions() const {
            return dims;
        }

        [[nodiscard]]
        size_type size() const {
            size_type ret = 1;

            for (int i = 0; i < dims.size(); ++i) {
                ret *= dims[i];
            }

            return ret;
        }

        //=================================================
        // Misc. methods
        //=================================================

        pointer data() {
            return ptr;
        }

        const_pointer data() const {
            return ptr;
        }

        //=================================================
        // Instance members
        //=================================================

    private:

        Ptr ptr;

        std::array<size_type, N> dims;

        //=================================================
        // Helper functions
        //=================================================

        size_type compute_offset(size_type s, dimension_type d) const {
            size_type ret = s;
            for (int i = 1; i < d.size(); ++i) {
                ret *= d[i];
            }
            return ret;
        }

    };



    ///
    /// \tparam T Element type
    /// \tparam N Number of dimensions
    /// \tparam A Allocator type
    template<class T, std::size_t N, class A = std::allocator<T>>
    class Matrix {
    public:

        static_assert(N > 0);
        static_assert(std::is_same_v<T, typename std::allocator_traits<A>::value_type>);

        //=================================================
        // Type aliases
        //=================================================

        using value_type = T;

        using reference = T&;
        using const_reference = const T&;

        using pointer = typename std::allocator_traits<A>::pointer;
        using const_pointer = typename std::allocator_traits<A>::const_pointer;

        using size_type = typename std::allocator_traits<A>::size_type;
        using difference_type = typename std::allocator_traits<A>::difference_type;

        using iterator = aul::Random_access_iterator<A, false>;
        using const_iterator = aul::Random_access_iterator<A, true>;

        using allocator_type = A;

        using dimension_type = std::array<size_type, N>;

    private:

        using alloc_traits = typename std::allocator_traits<A>;

        using lower_dimensional_view = Matrix_view<T, N - 1, A, false>;
        using const_lower_dimensional_ivew = Matrix_view<T, N - 1, A, true>;

        using subscript_return_type = std::conditional_t<
            N == 1,
            reference,
            lower_dimensional_view
        >;

        using const_subscript_return_type = std::conditional_t<
            N == 1,
            const_reference,
            const_lower_dimensional_ivew
        >;

    public:

        //=================================================
        // -ctors
        //=================================================

        Matrix() = default;

        explicit Matrix(const A& a):
            allocator(a) {}

        explicit Matrix(const dimension_type& dims):
            allocator(),
            dims(dims),
            ptr(allocate(dims)) {

            aul::default_construct_n(ptr, size(), allocator);
        }

        Matrix(const dimension_type& dims, const allocator_type& a):
            allocator(a),
            dims(dims),
            ptr(allocate(dims)) {

            aul::default_construct_n(ptr, size(), allocator);
        }

        ///
        /// Copy constructor
        ///
        /// \param matrix
        Matrix(const Matrix& matrix):
            allocator(alloc_traits::select_on_container_copy_construction(matrix.allocator)),
            dims(matrix.dims),
            ptr(allocate(dims)) {

            aul::uninitialized_copy_n(matrix.ptr, size(), ptr, allocator);
        }

        Matrix(const Matrix& matrix, const A& allocator):
            allocator(allocator),
            dims(matrix.dims),
            ptr(allocate(dims)) {

            aul::uninitialized_copy_n(matrix.ptr, size(), ptr, allocator);
        }

        Matrix(Matrix&& matrix) noexcept:
            allocator(std::move(matrix.allocator)),
            dims(std::move(matrix.dims)),
            ptr(matrix.ptr) {

            matrix.dims = {};
            matrix.ptr = nullptr;
        }

        Matrix(Matrix&& matrix, const A& allocator):
            allocator(allocator),
            dims(std::move(matrix.dims)),
            ptr((matrix.allocator == allocator) ? std::exchange(matrix.ptr, nullptr) : allocate(dims)) {

            if (!(matrix.allocator == allocator)) {
                aul::uninitialized_move_n(matrix.ptr, size(), ptr, allocator);
            }
        }

        ~Matrix() {
            clear();
        }

        //=================================================
        // Assignment operators
        //=================================================

        Matrix& operator=(const Matrix& matrix) {
            if constexpr (std::allocator_traits<A>::propagate_on_container_copy_assignment::value) {
                allocator = matrix.allocator;
            }

            dims = matrix.dims;
            ptr = allocate(dims);
            aul::uninitialized_copy_n(matrix.ptr, size(), ptr, allocator);

            return *this;
        }

        Matrix& operator=(Matrix&& matrix) noexcept {
            if constexpr (std::allocator_traits<A>::propagate_on_container_move_assignment::value) {
                allocator = std::move(matrix.allocator);
            }

            dims = std::exchange(matrix.dims, {});
            ptr = std::exchange(matrix.ptr, nullptr);

            return *this;
        }

        /*
        template<bool const_view>
        Matrix& operator=(Matrix_view<T, N, A, const_view>& mat) {
            resize(mat.dimensions());

            std::copy_n(mat.data(), mat.size(), data());

            return *this;
        }
        */

        //=================================================
        // Comparison operators
        //=================================================

        bool operator==(const Matrix& matrix) const {
            return
                (dims == matrix.dims) &&
                std::equal(begin(), end(), matrix.begin());
        }

        bool operator!=(const Matrix& matrix) const {
            return
                (dims != matrix.dims) ||
                !std::equal(begin(), end(), matrix.begin());
        }

        //=================================================
        // Iterator methods
        //=================================================

        iterator begin() {
            return iterator{ptr};
        }

        const_iterator begin() const {
            return const_iterator{ptr};
        }

        const_iterator cbegin() const {
            return const_cast<const Matrix&>(*this).begin();
        }

        iterator end() {
            return iterator{ptr + size()};
        }

        const_iterator end() const {
            return const_iterator{ptr + size()};
        }

        const_iterator cend() const {
            return const_cast<const Matrix&>(*this).begin();
        }

        //=================================================
        // Element accessors
        //=================================================

        subscript_return_type operator[](const size_type n) {
            if constexpr (N == 1) {
                return ptr[n];
            } else {
                return subscript_return_type{ptr + compute_offset(n, dims), dims.data() + 1};
            }
        }

        const_subscript_return_type operator[](const size_type n) const {
            if constexpr (N == 1) {
                return ptr[n];
            } else {
                return const_subscript_return_type{ptr + compute_offset(n, dims), dims.data() + 1};
            }
        }

        reference at(const dimension_type& pos) {
            for (std::size_t i = 0; i < N; ++i) {
                if (dims[i] <= pos[i]) {
                    throw std::out_of_range("Index out of range in call to aul::Matrix::at().");
                }
            }

            size_type offset = 0;
            size_type coefficient = 1;
            for (std::size_t i = N; i-- > 0;) {
                offset += (coefficient * pos[i]);
                coefficient *= dims[i];
            }

            return ptr[offset];
        }

        const_reference at(const dimension_type& pos) const {
            for (std::size_t i = 0; i < N; ++i) {
                if (dims[i] <= pos[i]) {
                    throw std::out_of_range("Index out of range in call to aul::Matrix::at().");
                }
            }

            size_type offset = 0;
            size_type coefficient = 1;
            for (std::size_t i = N; i-- > 0;) {
                offset += (coefficient * pos[i]);
                coefficient *= dims[i];
            }

            return ptr[offset];
        }

        //=================================================
        // Size methods
        //=================================================

        //TODO: Test and finish implementation
        /*
        void resize(const dimension_type& new_dimensions) {
            if (!dimension_safety(new_dimensions)) {
                throw std::length_error("Length error i call to aul::Matrix::resize(). Dimensions are too large to represent using container size type.");
            }

            pointer new_ptr = allocate(new_dimensions);

            Matrix_view<T, N, A> view{new_ptr, new_dimensions};
            dimension_type counters{};

            dimension_type bounds;
            for (int i = 0; i < bounds.size(); ++i) {
                bounds[i] = std::min(dims[i], new_dimensions[i]);
            }

            for (;counters != new_dimensions;) {
                //Copy elements

                view.at(counters) = this->at(counters);

                //Increment counters
                counters[counter.size() - 1] += 1;
                for (int i = counters.size(); i-- > 0;) {
                    if (counters[i] == bounds[i]) {
                        counters[i] = 0;
                        counters[i - 1] -= 1;
                    }
                }
            }
        }
        */

        void clear() {
            const size_type num_elems = size();
            for (size_type i = 0; i < num_elems; ++i) {
                std::allocator_traits<A>::destroy(allocator, ptr + i);
            }

            std::allocator_traits<A>::deallocate(allocator, ptr, num_elems);
            ptr = nullptr;
            dims.fill(0);
        }

        ///
        /// \return Number of elements in matrix.
        ///
        [[nodiscard]]
        size_type size() const {
            return std::reduce(dims.data(), dims.data() + N, 1, std::multiplies<size_type>{});
        }

        ///
        /// \return A std::array a=object containing the matrix's dimensions
        ///
        [[nodiscard]]
        dimension_type dimensions() const {
            return dims;
        }

        [[nodiscard]]
        bool empty() const {
            return !ptr;
        }

        //=================================================
        // Misc. methods
        //=================================================

        [[nodiscard]]
        allocator_type get_allocator() const {
            return allocator;
        }

        void swap(Matrix& matrix) {
            std::swap(allocator, matrix.allocator);
            std::swap(dims, matrix.dims);
            std::swap(ptr, matrix.ptr);
        }

        pointer data() {
            return ptr;
        }

        const_pointer data() const {
            return ptr;
        }

    private:

        //=================================================
        // Instance members
        //=================================================

        A allocator{};

        dimension_type dims{};

        pointer ptr = nullptr;

        //=================================================
        // Helper functions
        //=================================================

        pointer allocate(const dimension_type& dimensions) {
            size_type allocation_size = std::reduce(dimensions.data(), dimensions.data() + N, 1, std::multiplies<size_type>{});
            return std::allocator_traits<A>::allocate(allocator, allocation_size);
        }

        bool dimension_safety(const dimension_type& dimensions) {
            constexpr size_type max = std::numeric_limits<size_type>::max();

            size_type quotient = max / dimensions[0];

            for (std::size_t i = 1; i < dimensions.size(); ++i) {
                quotient /= dimensions[i];
            }

            return (quotient != 0);
        }

        size_type compute_offset(size_type s, dimension_type d) const {
            size_type ret = s;
            for (int i = 1; i < d.size(); ++i) {
                ret *= d[i];
            }
            return ret;
        }

    };

}

#endif //AUL_MATRIX_HPP

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
#include <limits>
#include <utility>

namespace aul {

    ///
    /// \tparam P Pointer type
    /// \tparam S Size_type. Used as parameter type for subscripting
    /// \tparam N Number of dimensions
    template<class P, class S, std::size_t N>
    class Matrix_view {
    public:

        static_assert(N > 0);

        //=================================================
        // Type aliases
        //=================================================

        using value_type = typename std::pointer_traits<P>::element_type;

        using reference = value_type&;

        using pointer = P;

        using size_type = S;
        using difference_type = typename std::pointer_traits<P>::difference_type;

        using iterator = aul::Random_access_iterator<pointer>;

        using dimension_type = std::array<size_type, N>;

    private:

        using lower_dimensional_view = std::conditional_t<
            N == 1,
            reference,
            Matrix_view<pointer, size_type, N - 1>
        >;

    public:

        //=================================================
        // -ctors
        //=================================================

        Matrix_view() = default;

        Matrix_view(pointer ptr, dimension_type dims):
            ptr(ptr),
            dims(std::move(dims)) {}

        Matrix_view(pointer ptr, const typename dimension_type::size_type* dim_ptr):
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

        lower_dimensional_view operator[](const size_type n) const {
            if constexpr (N == 1) {
                return ptr[n];
            } else {
                return lower_dimensional_view{ptr + compute_offset(n, dims), dims.data()};
            }
        }

        template<class...Args, class = typename aul::enable_if_homogenous_N_t<N + 1, size_type, Args...>>
        reference at(Args...args) const {
            return at(aul::array_from_T<N, size_type>(args...));
        }

        reference at(const std::array<size_type, N>& pos) const {
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

        pointer data() const {
            return ptr;
        }

        //=================================================
        // Instance members
        //=================================================

    private:

        pointer ptr{};

        std::array<size_type, N> dims;

        //=================================================
        // Helper functions
        //=================================================

        ///
        /// \param n Nth matri
        /// \param d
        /// \return
        size_type compute_offset(size_type n, dimension_type d) const {
            size_type ret = n;
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

        using iterator = aul::Random_access_iterator<pointer>;
        using const_iterator = aul::Random_access_iterator<const_pointer>;

        using allocator_type = A;

        using dimension_type = std::array<size_type, N>;

    private:

        using alloc_traits = typename std::allocator_traits<A>;

        using lower_dimensional_view = std::conditional_t<
            N == 1,
            reference,
            Matrix_view<pointer, size_type, N - 1>
        >;

        using const_lower_dimensional_ivew = std::conditional_t<
            N == 1,
            const_reference,
            Matrix_view<const_pointer, size_type, N - 1>
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
            allocation(allocate(dims)) {

            aul::default_construct_n(allocation, size(), allocator);
        }

        Matrix(const dimension_type& dims, value_type x):
            allocator(),
            dims(dims),
            allocation(allocate(dims)) {

            aul::uninitialized_fill_n(allocation, size(), x, allocator);
        }

        Matrix(const dimension_type& dims, const allocator_type& a):
            allocator(a),
            dims(dims),
            allocation(allocate(dims)) {

            aul::default_construct_n(allocation, size(), allocator);
        }

        ///
        /// Copy constructor
        ///
        /// \param matrix
        Matrix(const Matrix& matrix):
            allocator(alloc_traits::select_on_container_copy_construction(matrix.allocator)),
            dims(matrix.dims),
            allocation(allocate(dims)) {

            aul::uninitialized_copy_n(matrix.allocation, size(), allocation, allocator);
        }

        Matrix(const Matrix& matrix, const A& allocator):
            allocator(allocator),
            dims(matrix.dims),
            allocation(allocate(dims)) {

            aul::uninitialized_copy_n(matrix.allocation, size(), allocation, allocator);
        }

        Matrix(Matrix&& matrix) noexcept:
            allocator(std::move(matrix.allocator)),
            dims(std::move(matrix.dims)),
            allocation(matrix.allocation) {

            matrix.dims = {};
            matrix.allocation = nullptr;
        }

        Matrix(Matrix&& matrix, const A& allocator):
            allocator(allocator),
            dims(std::move(matrix.dims)),
            allocation((matrix.allocator == allocator) ? std::exchange(matrix.allocation, nullptr) : allocate(dims)) {

            if (!(matrix.allocator == allocator)) {
                aul::uninitialized_move_n(matrix.allocation, size(), allocation, allocator);
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
            allocation = allocate(dims);
            aul::uninitialized_copy_n(matrix.allocation, size(), allocation, allocator);

            return *this;
        }

        Matrix& operator=(Matrix&& matrix) noexcept {
            if constexpr (std::allocator_traits<A>::propagate_on_container_move_assignment::value) {
                allocator = std::move(matrix.allocator);
            }

            dims = std::exchange(matrix.dims, {});
            allocation = std::exchange(matrix.allocation, nullptr);

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
            return iterator{allocation};
        }

        const_iterator begin() const {
            return const_iterator{allocation};
        }

        const_iterator cbegin() const {
            return const_cast<const Matrix&>(*this).begin();
        }

        iterator end() {
            return iterator{allocation + size()};
        }

        const_iterator end() const {
            return const_iterator{allocation + size()};
        }

        const_iterator cend() const {
            return const_cast<const Matrix&>(*this).begin();
        }

        //=================================================
        // Element accessors
        //=================================================

        lower_dimensional_view operator[](const size_type n) {
            if constexpr (N == 1) {
                return allocation[n];
            } else {
                return lower_dimensional_view{allocation + compute_offset(n, dims), dims.data() + 1};
            }
        }

        const_lower_dimensional_ivew operator[](const size_type n) const {
            if constexpr (N == 1) {
                return allocation[n];
            } else {
                return const_lower_dimensional_ivew{allocation + compute_offset(n, dims), dims.data() + 1};
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

            return allocation[offset];
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

            return allocation[offset];
        }

        //=================================================
        // Size methods
        //=================================================

        //TODO: Test and finish implementation
        ///
        /// \param new_dimensions Dimensions of matrix after resizing
        /// \param v Value to fill in with if resizing produces empty cells
        void resize(const dimension_type& new_dimensions, const value_type& v = {}) {
            if (new_dimensions == dims) {
                return;
            }

            for (auto d : new_dimensions) {
                if (d == 0) {
                    clear();
                    return;
                }
            }

            if (!dimension_safety(new_dimensions)) {
                throw std::length_error("Length error in call to aul::Matrix::resize(). Dimensions are too large to represent using container size type.");
            }

            // Create new allocation
            pointer new_allocation = allocate(new_dimensions);

            //Copy overlapping elements
            pointer new_address = new_allocation;
            size_type num_elements = element_count(new_dimensions);
            dimension_type indices{};
            for (int i = 0; i < num_elements; ++i) {
                bool current_indices_in_old = true;
                for (std::size_t j = 0; j < indices.size(); ++j) {
                    current_indices_in_old &= (indices[j] < dims[j]);
                }

                // Move old element to new location if in old dimensions
                if (current_indices_in_old) {
                    // TODO: call to indices_to_offset may be eliminated and the
                    // work that it performed amortized
                    auto tmp = indices_to_offset(indices, dims);
                    pointer old_address = allocation + tmp;
                    std::allocator_traits<A>::construct(allocator, new_address, std::move(*old_address));
                } else {
                    std::allocator_traits<A>::construct(allocator, new_address, v);
                }

                new_address += 1;

                //Increment indices
                indices.back() += 1;
                for (std::size_t j = indices.size(); j-- > 1;) {
                    if (indices[j] == new_dimensions[j]) {
                        indices[j] = 0;
                        indices[j - 1] += 1;
                    }

                    // Branch can be eliminated. Following code doesn't work
                    /*
                    bool c = (indices[j] == new_dimensions[j]);

                    indices[j] &= -size_type(c);
                    indices[j - 1] += c;
                    */
                }
            }

            size_type old_allocation_size = element_count(dims);
            std::allocator_traits<A>::deallocate(allocator, allocation, old_allocation_size);
            allocation = new_allocation;
            dims = new_dimensions;
        }

        ///
        /// Resets dimensions of matrix to all zeroes.
        /// All elements are destroyed and current allocation is deallocated.
        ///
        void clear() {
            const size_type num_elems = size();
            for (size_type i = 0; i < num_elems; ++i) {
                std::allocator_traits<A>::destroy(allocator, allocation + i);
            }

            std::allocator_traits<A>::deallocate(allocator, allocation, num_elems);
            allocation = nullptr;
            dims.fill(0);
        }

        ///
        /// \return Number of elements in matrix.
        ///
        [[nodiscard]]
        size_type size() const {
            return element_count(dims);
        }

        ///
        /// \return A std::array object containing the matrix's dimensions
        ///
        [[nodiscard]]
        dimension_type dimensions() const {
            return dims;
        }

        ///
        /// \return Return true if dimensions are all zero
        [[nodiscard]]
        bool empty() const {
            // pointer is nullptr if and only if dimensions are all zero
            return !allocation;
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
            std::swap(allocation, matrix.allocation);
        }

        pointer data() {
            return allocation;
        }

        const_pointer data() const {
            return allocation;
        }

    private:

        //=================================================
        // Instance members
        //=================================================

        ///
        /// Matrix allocator object
        ///
        A allocator{};

        ///
        /// Matrix dimensions
        ///
        dimension_type dims{};

        ///
        /// Pointer to current allocation.
        /// Should be equal to nullptr if and only if dims is all zeroes
        ///
        pointer allocation = nullptr;

        //=================================================
        // Helper functions
        //=================================================

        ///
        /// \param dimensions Dimensions of matrix to allocate memory for
        /// \return Pointer to allocation large enough for matrix of specified
        ///     dimensions. Does not handle failure to allocate for any reason
        pointer allocate(const dimension_type& dimensions) {
            size_type allocation_size = element_count(dimensions);
            return std::allocator_traits<A>::allocate(allocator, allocation_size);
        }

        ///
        /// \param dimensions Matrix dimensions
        /// \return True is number of elements in matrix of specified dimensions
        ///     may be represented using size_type
        bool dimension_safety(const dimension_type& dimensions) const {
            constexpr size_type max = std::numeric_limits<size_type>::max();

            size_type quotient = max / dimensions[0];

            for (std::size_t i = 1; i < dimensions.size(); ++i) {
                quotient /= dimensions[i];
            }

            return (quotient != 0);
        }

        ///
        /// \param s
        /// \param d
        /// \return Offset into
        size_type compute_offset(size_type s, dimension_type d) const {
            size_type ret = s;

            for (std::size_t i = 1; i < d.size(); ++i) {
                ret *= d[i];
            }

            return ret;
        }

        ///
        /// \param indices Indices of matrix element
        /// \param dimensions Dimensions of matrices
        /// \return Offset into matrix's allocation for the specified indices
        static size_type indices_to_offset(dimension_type indices, dimension_type dimensions) {
            size_type ret = 0;

            size_type t = 1;
            for (std::size_t i = 0; i < N; ++i) {
                ret += (indices[N - i - 1] * t);
                t *= dimensions[i];
            }

            return ret;
        }

        ///
        /// \param d Matrix dimensions
        /// \return Number of elements in matrix of specified dimensions
        size_type element_count(dimension_type d) const {
            size_type ret = d[0];
            for (std::size_t i = 1; i < N; ++i) {
                ret *= d[i];
            }
            return ret;
        }

    };

}

#endif //AUL_MATRIX_HPP

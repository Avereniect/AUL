#ifndef AUL_LINEAR_ALLOCATOR_HPP
#define AUL_LINEAR_ALLOCATOR_HPP

#include <cstddef>
#include <cstdlib>
#include <memory>

namespace aul {

    class alignas(std::max_align_t) Linear_memory_block {
    public:

        //=================================================
        // Static methods
        //=================================================

        ///
        /// Allocators a block of memory block and constructs a header at
        /// its start.
        ///
        /// \param byte_count Number of bytes in memory block (excluding header)
        /// \return Pointer to memory block's header
        static Linear_memory_block* create(const std::size_t byte_count) {
            void* mem = malloc(byte_count + sizeof(Linear_memory_block));
            Linear_memory_block* block = new(mem) Linear_memory_block(byte_count);
            return block;
        }

        ///
        /// Frees memory associated with the Linear_memory_block object
        ///
        /// \param mem Header object of memory block
        static void destroy(Linear_memory_block* mem) {
            free(mem);
        }

        //=================================================
        // Deleted -ctors
        //=================================================

        Linear_memory_block(const Linear_memory_block&) = delete;
        Linear_memory_block(Linear_memory_block&&) = delete;

        //=================================================
        // Destructor
        //=================================================

        ~Linear_memory_block() = default;

        //=================================================
        // Deleted assignment operators
        //=================================================

        Linear_memory_block& operator=(const Linear_memory_block&) = delete;
        Linear_memory_block& operator=(Linear_memory_block&&) = delete;

        //=================================================
        // Comparison operators
        //=================================================

        bool operator==(const Linear_memory_block& block) {
            return this == &block;
        }

        bool operator!=(const Linear_memory_block& block) {
            return this != &block;
        }

        //=================================================
        // Accessors
        //=================================================

        [[nodiscard]]
        std::size_t capacity() const noexcept {
            return cap;
        }

        [[nodiscard]]
        std::byte* allocate(const std::size_t n, const std::size_t size, std::size_t alignment) {
            const std::size_t byte_count = n * size;
            if ( cap < byte_count || (cap - used_space()) < byte_count) {
                throw std::bad_alloc();
            }

            std::size_t m = cap - used_space();
            void* temp_ptr = head;
            void* aligned_temp = std::align(alignment, size, temp_ptr, m);

            //This may not be necessary
            if (!aligned_temp) {
                throw std::bad_alloc();
            }

            std::byte* return_value = reinterpret_cast<std::byte*>(aligned_temp);
            head += byte_count;

            return return_value;
        }

        void clear() noexcept {
            head = block_begin();
        }

        [[nodiscard]]
        std::size_t users() const noexcept {
            return usrs;
        }

        void increment_users() noexcept {
            ++usrs;
        }

        void decrement_users() noexcept {
            --usrs;
        }

    private:

        //=================================================
        // Constructors
        //=================================================

        explicit Linear_memory_block(const std::size_t n) :
            usrs(0),
            cap(n),
            head(block_begin()) {}

        //=================================================
        // Helper functions
        //=================================================

        [[nodiscard]]
        std::byte* block_begin() noexcept {
            return reinterpret_cast<std::byte*>(this) + sizeof(*this);
        }

        [[nodiscard]]
        const std::byte* block_begin() const noexcept {
            return reinterpret_cast<const std::byte*>(this) + sizeof(*this);
        }

        [[nodiscard]]
        std::size_t used_space() const noexcept {
            return head - block_begin();
        }

        //=================================================
        // Instance variables
        //=================================================

        std::size_t usrs = 0;

        std::size_t cap = 0;

        std::byte* head = nullptr;

    };

    ///
    /// An allocator offering cheap allocations.
    ///
    /// Memory is distributed from a memory pool that is shared between multiple
    /// Linear_allocator_objects. Constructing a Linear_allocator from another
    /// by any means causes the two to share their memory pools. Assigning one
    /// allocator to another causes the target allocator to let go of its
    /// current memory pool and take shared ownership of the source allocator's
    /// pool. A memory pool is freed once it has no more users.
    ///
    /// aul::Linear_allocator satisfies the STL allocator concept and thus can
    /// be used by all containers that satisfy the STL allocator-aware container
    /// concept.
    ///
    /// \tparam T Type to allocate memory for
    ///
    template<class T>
    class Linear_allocator {
    public:

        //=================================================
        // Type aliases
        //=================================================

        using value_type = T;

        template<class U>
        struct rebind {
            using other = Linear_allocator<U>;
        };

        //=================================================
        // Meta aliases
        //=================================================

        using propagate_on_container_copy_assignment = std::true_type;
        using propagate_on_container_move_assignment = std::true_type;
        using propagate_on_container_swap = std::true_type;
        using is_always_equal = std::false_type;

        //=================================================
        // -ctors
        //=================================================

        ///
        /// Constructs an allocator holding no memory pool.
        ///
        Linear_allocator() = default;

        ///
        /// Constructs an allocator holding a memory pool large enough to hold
        /// n many elements of type T.
        ///
        Linear_allocator(const std::size_t n) {
            block = Linear_memory_block::create(n * sizeof(T));
            block->increment_users();

            const char* name = typeid(T).name();
        }

        Linear_allocator(const Linear_allocator& alloc) noexcept :
            block(alloc.get_block()) {

            if (block) {
                block->increment_users();
            }

            const char* name = typeid(T).name();
        }

        ///
        ///
        ///
        /// \param alloc
        template<class U>
        Linear_allocator(const Linear_allocator<U>& alloc) noexcept:
            block(alloc.get_block()) {

            if (block) {
                block->increment_users();
            }
        }

        ///
        ///
        ///
        Linear_allocator(Linear_allocator&& alloc) noexcept:
            block(alloc.block) {

            if (block) {
               block->increment_users();
            }
        }

        ///
        ///
        ///
        ~Linear_allocator() {
            if (!block) {
                return;
            }
            block->decrement_users();

            if (!block->users()) {
                Linear_memory_block::destroy(block);
            }
        }

        //=================================================
        // Allocation & deallocation methods
        //=================================================

        ///
        /// Allocates memory from the internal block
        ///
        /// \param n     Number of elements to allocate memory for
        /// \return      Pointer to allocated block
        [[nodiscard]]
        T* allocate(const std::size_t n) {

            if (n == 0) {
                return nullptr;
            }

            if (!block) {
                throw std::bad_alloc();
            }

            const char* name = typeid(T).name();
            //std::cout << "Linear_allocator.allocate() called with type " << name << std::endl;

            auto size = sizeof(T);
            auto alignment = alignof(T);

            auto* temp  = reinterpret_cast<T*>(block->allocate(n, size, alignment));
            return temp;
        }

        ///
        /// \param n
        /// \param ptr
        /// \return
        [[nodiscard]]
        T* allocate(const std::size_t n, const void* ptr) {
            return allocate(n);
        }

        ///
        /// \param ptr Pointer to memory region to deallocate
        ///
        void deallocate(const T* ptr, const std::size_t n) noexcept {
            //Do literally nothing
            (void)n;
            (void)ptr;
        }

        ///
        /// Frees all memory consumed by objects in memory pool
        ///
        void clear() noexcept {
            block->clear();
        }

        //=================================================
        // Misc. methods
        //=================================================

        [[nodiscard]]
        std::size_t max_size() const noexcept {
            return block ? block->capacity() / sizeof(T) : 0;
        }

        //=================================================
        // Assignment operators
        //=================================================

        Linear_allocator& operator=(const Linear_allocator& alloc) noexcept {
            if (block) {
                block->decrement_users();
                if (!block->users()) {
                    Linear_memory_block::destroy(block);
                }
            }

            block = alloc.block;
            if (block) {
                block->increment_users();
            }

            return *this;
        }

        Linear_allocator& operator=(Linear_allocator&& alloc) noexcept {
            if (block) {
                block->decrement_users();
                if (!block->users()) {
                    Linear_memory_block::destroy(block);
                }
            }

            block = alloc.block;
            if (block) {
                block->increment_users();
            }

            return *this;
        }

        //=================================================
        // Comparison operators
        //=================================================

        bool operator==(const Linear_allocator& rhs) const {
            return block == rhs.block;
        }

        bool operator!=(const Linear_allocator& rhs) const {
            return block != rhs.block;
        }

        //=================================================
        // Misc. methods
        //=================================================

        Linear_memory_block* get_block() const noexcept {
            return block;
        }

    private:
        Linear_memory_block* block = nullptr;

    };

}

#endif

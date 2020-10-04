#ifndef AUL_STACK_ALLOCATOR_HPP
#define AUL_STACK_ALLOCATOR_HPP

#include <cstdint>
#include <cstddef>
#include <cstdlib>

namespace  aul {

    class alignas(std::max_align_t) Stack_memory_block {
    public:

        //=================================================
        // Helper classes
        //=================================================



        //=================================================
        // Static methods
        //=================================================

        static Stack_memory_block& create(const std::size_t byte_count) {
            void* mem = malloc(byte_count + sizeof(Stack_memory_block));
            Stack_memory_block* block = new(mem) Stack_memory_block(byte_count);
            return *block;
        }

        static void destroy(Stack_memory_block& block) {

        }

        //=================================================
        // Deleted -ctors
        //=================================================

        Stack_memory_block(const Stack_memory_block&) = delete;
        Stack_memory_block(Stack_memory_block&&) = delete;

        //=================================================
        // Destructor
        //=================================================

        ~Stack_memory_block() = default;

        //=================================================
        // Deleted assignment operators
        //=================================================

        Stack_memory_block& operator=(const Stack_memory_block&) = delete;
        Stack_memory_block& operator=(Stack_memory_block&&) = delete;

        //=================================================
        // Comparison operators
        //=================================================

        bool operator==(const Stack_memory_block& block) {
            return this == &block;
        }

        bool operator!=(const Stack_memory_block& block) {
            return this != &block;
        }

    private:

        Stack_memory_block(const std::size_t);

    };

    template<class T>
    class Stack_allocator {
    public:

        //=================================================
        // Type aliases
        //=================================================

        using value_type = T;

        template<class U>
        class rebind {
            using other = Stack_allocator<U>;
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
        /// Constructs a Stack_allocator holding no memory pool
        ///
        Stack_allocator() = default;

        ///
        /// Constructs a Stack_allocator with a newly allocated memory pool
        /// large enough to hold n elements. Allocation headers may reduce the 
        /// actual number of elements which can be held. 
        ///
        /// \param n Number of elements allocators must have memory for
        ///
        Stack_allocator(const std::size_t n) {

        }

        Stack_allocator(const Stack_allocator&) = default;
        Stack_allocator(Stack_allocator&&) = default;

        ~Stack_allocator() {

        }

        //=================================================
        // Assignment operators
        //=================================================




    private:
        Stack_memory_block* block = nullptr;

    };

}

#endif //AUL_STACK_ALLOCATOR_HPP

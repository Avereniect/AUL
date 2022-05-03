//
// Created by avereniect on 2/23/22.
//

#ifndef AUL_MEMORY_MAPPED_ALLOCATOR_HPP
#define AUL_MEMORY_MAPPED_ALLOCATOR_HPP

#ifndef __linux__
static_assert(false, "OS not supported");
#endif

#include <sys/mman.h>

#include <cstdint>

namespace aul {

    template<class T>
    class Memory_mapped_allocator {
    public:

        //=================================================
        // Type aliases
        //=================================================

        using value_type = T;

        using pointer = T*;
        using const_pointer = const T*;

        using void_pointer = void*;
        using const_void_pointer = const void*;

        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        //=================================================
        // -ctors
        //=================================================

        Memory_mapped_allocator();

        ~Memory_mapped_allocator() {
            if (ptr) {
                munmap(ptr, allocation_size);
            }
        }

        //=================================================
        // Assignment operators
        //=================================================

        //=================================================
        // Allocation methods
        //=================================================

        //=================================================
        // Object creation methods
        //=================================================

        //=================================================
        // Instance members
        //=================================================

    private:

        int file_descriptor = -1;
        size_type allocation_size = 0;
        pointer ptr;

    };

}

#endif //AUL_MEMORY_MAPPED_ALLOCATOR_HPP

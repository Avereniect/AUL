#ifndef AUL_DEBUG_ALLOCATOR_HPP
#define AUL_DEBUG_ALLOCATOR_HPP

#include <memory>

namespace aul {

    ///
    /// Wrapper around an STL allocator that keeps track of the number of
    /// memory allocation made with wrapped allocator. 
    /// 
    /// 
    ///
    template<class Alloc>
    class Debug_allocator : public Alloc {
    private:

        using base_pointer = typename std::allocator_traits<Alloc>::pointer;
        using base_size_type = typename std::allocator_traits<Alloc>::size_type;
        using base_const_void_pointer = typename std::allocator_traits<Alloc>::const_void_pointer;

    public:

        //=================================================
        // Type aliases
        //=================================================

        template<typename>
        using pointer = typename Alloc::pointer;

        template<typename>
        using const_pointer = typename Alloc::const_pointer;

        template<typename>
        using void_pointer = typename Alloc::void_pointer;

        template<typename>
        using const_void_pointer = typename Alloc::const_void_pointer;

        using value_type = typename Alloc::value_type;

        template<typename>
        using size_type = typename Alloc::size_type;

        template<typename>
        using difference_type = typename Alloc::difference_type;

        template<typename U>
        struct rebind {
            using other = Debug_allocator<U>;
        };

        //=================================================
        // Allocation methods
        //=================================================

        base_pointer allocate(const base_size_type n) {
            allocs_count += 1;

            return Alloc::allocate(n);
        }

        base_pointer allocate(const base_size_type n, const base_const_void_pointer ptr) {
            allocs_count += 1;

            return Alloc::allocate(n, ptr);
        }

        void deallocate(const base_pointer ptr, const base_size_type n) {
            allocs_count -= 1;

            Alloc::dealloacte(ptr, n);
        }

        template<typename...Args>
        void construct(const base_pointer ptr, Args...args) {
            object_count += 1;

            Alloc::construct(ptr, std::forward<Args>(args)...);
        }

        template<typename...Args>
        void destroy(const base_pointer ptr) {
            object_count -= 1;

            Alloc::destroy(ptr);
        }

    private:
        std::size_t allocs_count = 0;
        std::size_t object_count = 0;

    };

}

#endif

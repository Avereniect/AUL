//
// Created by avereniect on 8/20/20.
//

#ifndef AUL_MEMORY_POOL_HPP
#define AUL_MEMORY_POOL_HPP

#include <cstdlib>
#include <memory>
#include <cstdint>

namespace aul {

    ///
    ///
    ///
    class Memory_pool {
    public:

        //=================================================
        // -ctors
        //=================================================

        Memory_pool() = default;

        Memory_pool(const std::size_t s):
            ptr(reinterpret_cast<std::byte*>(std::malloc(s))),
            byte_count(s),
            user_count(0) {}

        Memory_pool(const Memory_pool&) = delete;

        Memory_pool(Memory_pool&& pool):
            ptr(pool.ptr),
            byte_count(pool.byte_count),
            user_count(0) {

            ptr = nullptr;
            byte_count = 0;
        }

        ~Memory_pool() {
            free(ptr);
        }

        //=================================================
        // Assignment operators
        //=================================================

        Memory_pool& operator=(const Memory_pool&) = delete;

        Memory_pool& operator=(Memory_pool&& pool) {
            ptr = pool.ptr;
            byte_count = pool.byte_count;

            pool.ptr = nullptr;
            pool.byte_count = 0;

            return *this;
        }

        //=================================================
        // Mutators
        //=================================================

        void increment_users() {
            ++user_count;
        }

        void decrement_users() {
            --user_count;
        }

        std::size_t users() {
            return user_count;
        }

        //=================================================
        // Accessors
        //=================================================

        [[nodiscard]]
        std::byte* data() {
            return ptr;
        }

        std::size_t capacity() const {
            return byte_count;
        }

    private:

        //=================================================
        // Instance members
        //=================================================

        std::byte* ptr = nullptr;
        std::size_t byte_count = 0;
        std::size_t user_count = 0;
    };

}

#endif //AUL_MEMORY_POOL_HPP

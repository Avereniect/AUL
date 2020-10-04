//
// Created by avereniect on 9/18/20.
//

#ifndef AUL_BUFFER_VIEW_HPP
#define AUL_BUFFER_VIEW_HPP

#include <cstdint>
#include <memory>

namespace aul {

    ///
    /// A non-owning memory resource wrapper around a pre-existing segment of
    /// memory.
    ///
    class Buffer_view {
    public:

        //=================================================
        // -cotrs
        //=================================================

        Buffer_view(void* ptr, const std::size_t n) noexcept:
            ptr(reinterpret_cast<std::byte*>(ptr)),
            cap(n) {}

        Buffer_view(const Buffer_view&) = delete;

        Buffer_view(Buffer_view&& view):
            ptr(view.ptr),
            cap(view.cap) {

            view.ptr = nullptr;
            view.cap = 0;
        }

        ~Buffer_view() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Buffer_view& operator=(const Buffer_view&) = delete;

        Buffer_view& operator=(Buffer_view&& view) noexcept {
            ptr = view.ptr;
            cap = view.cap;

            view.ptr = nullptr;
            view.cap = 0;

            return *this;
        }

        //=================================================
        // Accessors
        //=================================================

        std::byte* data() {
            return ptr;
        }

        std::size_t capacity() {
            return cap;
        }

    private:

        std::byte* ptr = nullptr;
        std::size_t cap = 0;

    };

}

#endif //AUL_BUFFER_VIEW_HPP

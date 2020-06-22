//
// Created by avereniect on 4/27/20.
//

#ifndef AUL_POINTERS_HPP
#define AUL_POINTERS_HPP

#include "Algorithms.hpp"

namespace aul {

    ///
    /// \tparam P Pointer type. Must specialize std::remove_pointer
    /// \tparam D
    /// \tparam Readable Can reference be written to?
    /// \tparam Writable Can reference be read from?
    template<class P, class D = std::default_delete<T>, bool Readable, bool Writable>
    class Unique_ref {
    public:

        //TODO: Use std::is_empty

        //=================================================
        // Type aliases
        //=================================================

        using pointer = typename decltype(std::remove_reference<D>::type::pointer);
        using element_type = typename std::remove_pointer<P>;
        using deleter_type = D;

        //=================================================
        // -ctors
        //=================================================

        Unique_ref() noexcept;
        Unique_ref(const nullptr_t) noexcept;

        //=================================================
        // Conversion operators
        //=================================================

        operator bool() const noexcept;

    private:
    };

}

#endif //AUL_POINTERS_HPP

//
// Created by avereniect on 7/5/20.
//

#ifndef AUL_UTILITY_HPP
#define AUL_UTILITY_HPP

#include <cstdarg>
#include <type_traits>

namespace aul {

    template<class...T>
    constexpr bool are_equal(T&&...values) {
        return (values == ...);
    }

    template<>
    constexpr bool are_equal<>() {
        return true;
    }

    template<class T, class...U>
    struct first_type {
        using type = T;
    };

    template<class T, class...U>
    using first_type_t = typename first_type<T, U...>::type;

    template<class T, class...Args>
    class are_same_types : public std::bool_constant<
        std::is_same_v<T, first_type<Args...>::type> && are_same_types<Args...>::value
    > {};

    template<class T>
    class are_same_types<T> : public std::bool_constant<true> {};

    template<class...Args>
    constexpr static bool are_same_types_v = are_same_types<Args...>::value;
}

#endif //AUL_UTILITY_HPP

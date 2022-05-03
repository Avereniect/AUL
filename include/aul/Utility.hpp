//
// Created by avereniect on 7/5/20.
//

#ifndef AUL_UTILITY_HPP
#define AUL_UTILITY_HPP

#include <cstdarg>
#include <type_traits>
#include <array>
#include <limits>

namespace aul {

    template<class Int, class...Ints>
    struct widest_int {
    private:

        static constexpr bool c =
            std::numeric_limits<Int>::digits >
            std::numeric_limits<typename widest_int<Ints...>::type>::digits;

    public:

        using type = typename std::conditional_t<c, Int, typename widest_int<Ints...>::type>;

    };

    template<class Int>
    struct widest_int<Int> {
        static_assert(std::is_integral_v<Int> && std::is_signed_v<Int>);
        using type = Int;
    };

    template<class...Ints>
    using widest_int_t = typename widest_int<Ints...>::type;


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

    template<class...T>
    using first_type_t = typename first_type<T...>::type;



    template<class...Args>
    struct last_type {

        template<class T>
        struct identity {using type = T;};

        using type = typename decltype((identity<Args>{}, ...))::type;
    };

    template<class...Args>
    using last_type_t = typename last_type<Args...>::type;



    template<class T = void, class...Args>
    struct are_same_types : public std::bool_constant<
        std::is_same_v<T, typename first_type<Args...>::type> && are_same_types<Args...>::value
    > {};

    template<class T>
    struct are_same_types<T> : public std::bool_constant<true> {};

    template<class...Args>
    constexpr static bool are_same_types_v = are_same_types<Args...>::value;



    template<class T, class...Args>
    struct are_convertible_to : public std::bool_constant<
        std::is_convertible_v<T, last_type_t<Args...>> && are_convertible_to<Args...>::value
    > {};

    template<class T, class U>
    struct are_convertible_to<T, U> : public std::bool_constant<std::is_convertible_v<T, U>> {};

    template<class...Args>
    constexpr static bool are_convertible_to_v = are_convertible_to<Args...>::value;


    template<std::size_t N, class...Args>
    struct is_homogenous_N : public std::bool_constant<
        aul::are_same_types_v<Args...> && (sizeof...(Args) == N)
    > {};

    template<std::size_t N, class...Args>
    constexpr static bool is_homogenous_N_v = is_homogenous_N<N, Args...>::value;



    template<std::size_t N, class...Args>
    using enable_if_homogenous_N = std::enable_if_t<aul::is_homogenous_N_v<N, Args...>>;

    template<std::size_t N, class...Args>
    using enable_if_homogenous_N_t = enable_if_homogenous_N<N, Args...>;



    ///
    /// Inserts arguments into an object of std::array with size equal to the
    /// number of arguments passed to the function
    ///
    /// \tparam N Number of arguments to function
    /// \tparam Args Uniform list of types.
    /// \param args Objects to pack into array
    /// \return A std::array object containing the parameters passed to the function
    template<std::size_t N, class...Args>
    [[nodiscard]]
    auto array_from(Args...args) {
        static_assert(N != 0);
        static_assert(sizeof...(Args) == N);
        static_assert(are_convertible_to_v<std::decay_t<Args>..., std::decay_t<first_type_t<Args...>>>);

        using U = typename std::decay_t<first_type_t<Args...>>;

        std::array<U, N> ret{static_cast<U>(args)...};
        return ret;
    }

    template<std::size_t N, class T, class...Args>
    [[nodiscard]]
    auto array_from_T(Args...args) {
        static_assert(N != 0);
        static_assert(sizeof...(Args) == N);
        static_assert(are_convertible_to_v<std::decay_t<Args>..., T>);

        std::array<T, N> ret{static_cast<T>(args)...};
        return ret;
    }



    namespace {

        template<std::size_t N, class T, class U>
        struct Array_and_value {
            std::array<T, N> arr;
            U x;
        };

    }

    template<std::size_t N, class T, class U, class...Args>
    [[nodiscard]]
    std::pair<std::array<T, N>, U> array_and_value(Args...args) {
        Array_and_value<N, T, U> ret = {args...};
        return std::pair<std::array<T, N>, U>{ret.arr, ret.x};
    }

}

#endif //AUL_UTILITY_HPP

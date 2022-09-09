//
// Created by avereniect on 1/9/22.
//

#ifndef AUL_ZIPPER_ITERATOR_HPP
#define AUL_ZIPPER_ITERATOR_HPP

#include "../Utility.hpp"

#include <tuple>
#include <iterator>
#include <utility>

namespace aul {

    namespace impl {

        template<class It, class...Its>
        auto dereference(It it, Its...its) {
            return std::tuple_cat(std::make_tuple(*it), dereference(its...));
        }

        template<class It0, class It1>
        auto dereference(It0 it0, It1 it1) {
            return std::tie(*it0, *it1);
        }



        template<class It, class...Its>
        auto arrow(It it, Its...its) {
            return std::tuple_cat(std::make_tuple(it.operator->()), arrow(its...));
        }

        template<class It0, class It1>
        auto arrow(It0 it0, It1 it1) {
            return std::tie(arrow(it0), arrow(it1));
        }

        template<class T>
        auto arrow(T* ptr) {
            return ptr;
        }

        template<class It>
        auto arrow(It it) {
            return it.operator->();
        }

    }

    ///
    /// Wrapper around individual iterators that functions as a parallel
    /// iterator. Functions purely as a forward iterator
    ///
    /// \tparam It First iterator types
    /// \tparam Its Other iterator types
    template<class It, class...Its>
    class Forward_zipperator {
    public:

        static_assert(
            sizeof...(Its),
            "Attempted to create a zipperator that does not abstract over "
            "multiple iterators"
        );

        static_assert(
            std::is_same<typename std::iterator_traits<It>::iterator_category, std::forward_iterator_tag>::value ||
            std::is_same<typename std::iterator_traits<It>::iterator_category, std::bidirectional_iterator_tag>::value ||
            std::is_same<typename std::iterator_traits<It>::iterator_category, std::random_access_iterator_tag>::value,
            "Iterator is not of required iterator category"
        );

        //=================================================
        // Type aliases
        //=================================================

    private:

        using sub_iterator = Forward_zipperator<Its...>;

    public:

        using value_type = std::tuple<
            typename std::iterator_traits<It>::value_type,
            typename std::iterator_traits<Its>::value_type...
        >;

        using difference_type = aul::widest_int_t<
            typename std::iterator_traits<It>::difference_type,
            typename std::iterator_traits<Its>::difference_type...
        >;

        using pointer = std::tuple<
            typename std::iterator_traits<It>::pointer,
            typename std::iterator_traits<Its>::pointer...
        >;

        using reference = std::tuple<
            typename std::iterator_traits<It>::reference,
            typename std::iterator_traits<Its>::reference...
        >;

        using iterator_category = std::forward_iterator_tag;

        //=================================================
        // -ctors
        //=================================================

        explicit Forward_zipperator(It it, Its...its):
            it(it),
            its(its...) {}

        Forward_zipperator() = default;
        Forward_zipperator(const Forward_zipperator&) = default;
        Forward_zipperator(Forward_zipperator&&) noexcept = default;
        ~Forward_zipperator() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Forward_zipperator& operator=(const Forward_zipperator&) = default;
        Forward_zipperator& operator=(Forward_zipperator&&) noexcept = default;

        //=================================================
        // Comparison operators
        //=================================================

        bool operator==(const Forward_zipperator& rhs) const {
            return (it == rhs.it0) && (its == rhs.its);
        }

        bool operator!=(const Forward_zipperator& rhs) const {
            return (it != rhs.it0) || (its != rhs.its);
        }

        //=================================================
        // Increment operators
        //=================================================

        Forward_zipperator& operator++() {
            ++it;
            ++its;
            return *this;
        }

        Forward_zipperator operator++(int) {
            auto tmp = *this;
            ++it;
            ++its;
            return tmp;
        }

        //=================================================
        // Dereference operators
        //=================================================

        reference operator*() const {
            return std::tuple_cat(
                std::make_tuple(*it),
                *its
            );
        }

        pointer operator->() const {
            return std::tuple_cat(
                std::make_tuple(impl::arrow(it)),
                its.operator->()
            );
        }

        //template<class It0_2, class...Args2, class = typename std::enable_if<>::type>
        //operator Forward_zipperator<const It0_2, Args2...>() const {//TODO: Implement}

    protected:

        //=================================================
        // Instance members
        //=================================================

        It it;
        sub_iterator its;

    };

    ///
    /// Specialization of Forward_zipperator for a single iterator
    ///
    /// \tparam It Iterator type
    template<class It0, class It1>
    class Forward_zipperator<It0, It1> {
    public:

        static_assert(
            std::is_same<typename std::iterator_traits<It0>::iterator_category, std::forward_iterator_tag>::value ||
            std::is_same<typename std::iterator_traits<It0>::iterator_category, std::bidirectional_iterator_tag>::value ||
            std::is_same<typename std::iterator_traits<It0>::iterator_category, std::random_access_iterator_tag>::value,
            "Iterator is not of required iterator category"
        );

        static_assert(
            std::is_same<typename std::iterator_traits<It1>::iterator_category, std::forward_iterator_tag>::value ||
            std::is_same<typename std::iterator_traits<It1>::iterator_category, std::bidirectional_iterator_tag>::value ||
            std::is_same<typename std::iterator_traits<It1>::iterator_category, std::random_access_iterator_tag>::value,
            "Iterator is not of required iterator category"
        );

        //=================================================
        // Type aliases
        //=================================================

        using value_type = std::tuple<
            typename std::iterator_traits<It0>::value_type,
            typename std::iterator_traits<It1>::value_type
        >;

        using difference_type = aul::widest_int_t<
            typename std::iterator_traits<It0>::difference_type,
            typename std::iterator_traits<It1>::difference_type
        >;

        using pointer = std::tuple<
            typename std::iterator_traits<It0>::pointer,
            typename std::iterator_traits<It1>::pointer
        >;

        using reference = std::tuple<
            typename std::iterator_traits<It0>::reference,
            typename std::iterator_traits<It1>::reference
        >;

        using iterator_category = std::forward_iterator_tag;

        //=================================================
        // -ctors
        //=================================================

        explicit Forward_zipperator(It0 it0, It1 it1):
            it0(it0),
            it1(it1) {}

        Forward_zipperator() = default;
        Forward_zipperator(const Forward_zipperator&) = default;
        Forward_zipperator(Forward_zipperator&&) noexcept = default;
        ~Forward_zipperator() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Forward_zipperator& operator=(const Forward_zipperator&) = default;
        Forward_zipperator& operator=(Forward_zipperator&&) noexcept = default;

        //=================================================
        // Comparison operators
        //=================================================

        bool operator==(const Forward_zipperator& rhs) const {
            return (it0 == rhs.it0) && (it1 == rhs.it1);
        }

        bool operator!=(const Forward_zipperator& rhs) const {
            return (it0 != rhs.it0) || (it1 != rhs.it1);
        }

        //=================================================
        // Increment operators
        //=================================================

        Forward_zipperator& operator++() {
            ++it0;
            ++it1;
            return *this;
        }

        Forward_zipperator operator++(int) {
            auto tmp = *this;
            ++it0;
            ++it1;
            return tmp;
        }

        //=================================================
        // Dereference operators
        //=================================================

        reference operator*() const {
            return std::tie(*it0, *it1);
        }

        pointer operator->() const {
            return std::make_tuple(impl::arrow(it0), impl::arrow(it1));
        }

    protected:

        //=================================================
        // Instance members
        //=================================================

        It0 it0;
        It1 it1;

    };

    template<class It, class...Its>
    class Bidirectional_zipperator : public Bidirectional_zipperator<Its...> {
        using base = Bidirectional_zipperator<Its...>;
    public:

        static_assert(
            sizeof...(Its),
            "Attempted to create a zipperator that does not abstract over "
            "multiple iterators"
        );

        static_assert(
            std::is_same<typename std::iterator_traits<It>::iterator_category, std::bidirectional_iterator_tag>::value ||
            std::is_same<typename std::iterator_traits<It>::iterator_category, std::random_access_iterator_tag>::value,
            "Iterator is not of required iterator category"
        );

        //=================================================
        // Type aliases
        //=================================================

        using value_type = typename std::iterator_traits<It>::value_type;
        using difference_type = typename std::iterator_traits<It>::difference_type;
        using pointer = typename std::iterator_traits<It>::pointer;
        using reference = typename std::iterator_traits<It>::reference;
        using iterator_category = std::bidirectional_iterator_tag;

        //=================================================
        // -ctors
        //=================================================

        explicit Bidirectional_zipperator(It it, Its...its):
            base(it),
            its(its...) {}

        Bidirectional_zipperator() = default;
        Bidirectional_zipperator(const Bidirectional_zipperator&) = default;
        Bidirectional_zipperator(Bidirectional_zipperator&&) noexcept = default;
        ~Bidirectional_zipperator() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Bidirectional_zipperator& operator=(const Bidirectional_zipperator&) = default;
        Bidirectional_zipperator& operator=(Bidirectional_zipperator&&) noexcept = default;

        //=================================================
        // Decrement_operators
        //=================================================

        Bidirectional_zipperator& operator--() {
            --base::it0;
            return *this;
        }

        Bidirectional_zipperator operator--(int) {
            auto tmp = *this;
            --base::it0;
            return tmp;
        }

    private:

        //=================================================
        // Instance members
        //=================================================

        Bidirectional_zipperator<Its...> its;

    };

    template<class It0, class It1>
    class Bidirectional_zipperator<It0, It1> : public Forward_zipperator<It0, It1> {
        using base = Forward_zipperator<It0, It1>;
    public:

        static_assert(
            std::is_same<typename std::iterator_traits<It0>::iterator_category, std::bidirectional_iterator_tag>::value ||
            std::is_same<typename std::iterator_traits<It0>::iterator_category, std::random_access_iterator_tag>::value,
            "Iterator is not of required iterator category"
        );

        static_assert(
            std::is_same<typename std::iterator_traits<It1>::iterator_category, std::bidirectional_iterator_tag>::value ||
            std::is_same<typename std::iterator_traits<It1>::iterator_category, std::random_access_iterator_tag>::value,
            "Iterator is not of required iterator category"
        );

        //=================================================
        // Type aliases
        //=================================================

        using value_type = typename base::value_type;
        using difference_type = typename base::difference_type;
        using pointer = typename base::pointer;
        using reference = typename base::reference;

        using iterator_category = std::bidirectional_iterator_tag;

        //=================================================
        // -ctors
        //=================================================

        explicit Bidirectional_zipperator(It0 it0, It1 it1):
            base(it0, it1) {}

        Bidirectional_zipperator() = default;
        Bidirectional_zipperator(const Bidirectional_zipperator&) = default;
        Bidirectional_zipperator(Bidirectional_zipperator&&) noexcept = default;
        ~Bidirectional_zipperator() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Bidirectional_zipperator& operator=(const Bidirectional_zipperator&) = default;
        Bidirectional_zipperator& operator=(Bidirectional_zipperator&&) = default;

        //=================================================
        // Decrement_operators
        //=================================================

        Bidirectional_zipperator& operator--() {
            --base::it;
            return *this;
        }

        Bidirectional_zipperator operator--(int) {
            auto tmp = *this;
            --base::it;
            return tmp;
        }

    };

    template<class It, class...Its>
    class Random_access_zipperator : public Random_access_zipperator<Its...> {
        using base = Random_access_zipperator<Its...>;
    public:

        static_assert(
            sizeof...(Its),
            "Attempted to create a zipperator that does not abstract over "
            "multiple iterators"
        );

        static_assert(
            std::is_same<typename std::iterator_traits<It>::iterator_category, std::random_access_iterator_tag>::value,
            "Iterator is not of required iterator category"
        );

        //=================================================
        // Type aliases
        //=================================================

        using value_type = std::tuple<
            typename std::iterator_traits<It>::value_type,
            typename std::iterator_traits<Its>::value_type...
        >;

        using difference_type = aul::widest_int_t<
            typename std::iterator_traits<It>::difference_type,
            typename std::iterator_traits<Its>::difference_type...
        >;

        using pointer = std::tuple<
            typename std::iterator_traits<It>::pointer,
            typename std::iterator_traits<Its>::pointer...
        >;

        using reference = std::tuple<
            typename std::iterator_traits<It>::reference,
            typename std::iterator_traits<Its>::reference...
        >;

        using iterator_category = std::random_access_iterator_tag;

        //=================================================
        // -ctors
        //=================================================

        explicit Random_access_zipperator(It it, Its... its):
            base(its...),
            it(it) {}

        Random_access_zipperator() = default;
        Random_access_zipperator(const Random_access_zipperator&) = default;
        Random_access_zipperator(Random_access_zipperator&&) noexcept = default;
        ~Random_access_zipperator() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Random_access_zipperator& operator=(const Random_access_zipperator&) = default;
        Random_access_zipperator& operator=(Random_access_zipperator&&) noexcept = default;

        //=================================================
        // Comparison operators
        //=================================================

        bool operator<(const Random_access_zipperator& rhs) {
            return (base::it < rhs.it);
        }

        bool operator>(const Random_access_zipperator& rhs) {
            return (base::it > rhs.it);
        }

        bool operator>=(const Random_access_zipperator& rhs) {
            return (base::it >= rhs.it);
        }

        bool operator<=(const Random_access_zipperator& rhs) {
            return (base::it <= rhs.it);
        }

        //=================================================
        // Arithmetic assignment operators
        //=================================================

        Random_access_zipperator& operator+=(difference_type d) {
            base::operator+=(d);
            it += d;
            return *this;
        }

        Random_access_zipperator& operator-=(difference_type d) {
            base::operator-=(d);
            it -= d;
            return *this;
        }

        //=================================================
        // Arithmetic operators
        //=================================================

        Random_access_zipperator operator+(difference_type rhs) const {
            auto ret = *this;
            ret.it += rhs;
            ret.base::operator+=(rhs);
            return ret;
        }

        friend Random_access_zipperator operator+(difference_type lhs, Random_access_zipperator rhs) {
            return (rhs + lhs);
        }

        Random_access_zipperator operator-(difference_type rhs) const {
            auto ret = *this;
            ret.it0 -= rhs;
            ret.base::operator-=(rhs);
            return ret;
        }

        friend Random_access_zipperator operator-(difference_type lhs, Random_access_zipperator rhs) {
            return (rhs - lhs);
        }

        friend difference_type operator-(Random_access_zipperator lhs, Random_access_zipperator rhs) {
            return lhs.it - rhs.it;
        }

        //=================================================
        // Dereference operators
        //=================================================

        reference operator*() const {
            return std::tuple_cat(
                std::tie(*it),
                base::operator*()
            );
        }

        reference operator[](difference_type d) const {
            auto tmp = *this;
            tmp += d;
            return *tmp;
        }

    private:

        //=================================================
        // Instance members
        //=================================================

        It it;

    };

    template<class It0, class It1>
    class Random_access_zipperator<It0, It1> : public Bidirectional_zipperator<It0, It1> {
        using base = Bidirectional_zipperator<It0, It1>;
    public:

        static_assert(
            std::is_same<typename std::iterator_traits<It0>::iterator_category, std::random_access_iterator_tag>::value,
            "Iterator is not of required iterator category"
        );

        static_assert(
            std::is_same<typename std::iterator_traits<It1>::iterator_category, std::random_access_iterator_tag>::value,
            "Iterator is not of required iterator category"
        );

        //=================================================
        // Type aliases
        //=================================================

        using value_type = typename base::value_type;
        using difference_type = typename base::difference_type;
        using pointer = typename base::pointer;
        using reference = typename base::reference;
        using iterator_category = std::random_access_iterator_tag;

        //=================================================
        // -ctors
        //=================================================

        explicit Random_access_zipperator(It0 it0, It1 it1):
            base(it0, it1) {}

        Random_access_zipperator() = default;
        Random_access_zipperator(const Random_access_zipperator&) = default;
        Random_access_zipperator(Random_access_zipperator&&) noexcept = default;
        ~Random_access_zipperator() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Random_access_zipperator& operator=(const Random_access_zipperator&) = default;
        Random_access_zipperator& operator=(Random_access_zipperator&&) noexcept = default;

        //=================================================
        // Comparison operators
        //=================================================

        bool operator<(const Random_access_zipperator& rhs) const {
            return (base::it0 < rhs.it0);
        }

        bool operator>(const Random_access_zipperator& rhs) const {
            return (base::it0 > rhs.it0);
        }

        bool operator>=(const Random_access_zipperator& rhs) const {
            return (base::it0 >= rhs.it0);
        }

        bool operator<=(const Random_access_zipperator& rhs) const {
            return (base::it0 <= rhs.it0);
        }

        //=================================================
        // Arithmetic assignment operators
        //=================================================

        Random_access_zipperator& operator+=(difference_type d) {
            base::it0 += d;
            base::it1 += d;
            return *this;
        }

        Random_access_zipperator& operator-=(difference_type d) {
            base::it0 -= d;
            base::it1 -= d;
            return *this;
        }

        //=================================================
        // Arithmetic operators
        //=================================================

        Random_access_zipperator operator+(difference_type rhs) const {
            auto ret = *this;
            ret.it0 += rhs;
            ret.it1 += rhs;
            return ret;
        }

        friend Random_access_zipperator operator+(difference_type lhs, Random_access_zipperator rhs) {
            return (rhs + lhs);
        }

        Random_access_zipperator operator-(difference_type rhs) const {
            auto ret = *this;
            ret.it0 -= rhs;
            ret.it1 -= rhs;
            return ret;
        }

        friend difference_type operator-(Random_access_zipperator lhs, Random_access_zipperator rhs) {
            return lhs.it0 - rhs.it0;
        }

        //=================================================
        // Dereference operators
        //=================================================

        reference operator[](difference_type d) const {
            auto tmp = *this;
            tmp += d;
            return *tmp;
        }

    };

}

namespace std {

    //=====================================================
    // std::tuple_size specializations
    //=====================================================

    template<class It, class...Args>
    struct tuple_size<aul::Forward_zipperator<It, Args...>> {
        static constexpr std::size_t value = 1 + sizeof...(Args);
    };

    template<class It, class...Args>
    struct tuple_size<aul::Bidirectional_zipperator<It, Args...>> {
        static constexpr std::size_t value = 1 + sizeof...(Args);
    };

    template<class It, class...Args>
    struct tuple_size<aul::Random_access_zipperator<It, Args...>> {
        static constexpr std::size_t value = 1 + sizeof...(Args);
    };

    //=====================================================
    // std::tuple_element specializations
    //=====================================================

    template<std::size_t index, class It, class...Args>
    struct tuple_element<index, aul::Forward_zipperator<It, Args...>> {
        using type = typename std::tuple_element<index, std::tuple<It, Args...>>::type;
    };

    template<std::size_t index, class It, class...Args>
    struct tuple_element<index, aul::Bidirectional_zipperator<It, Args...>> {
        using type = typename std::tuple_element<index, std::tuple<It, Args...>>::type;
    };

    template<std::size_t index, class It, class...Args>
    struct tuple_element<index, aul::Random_access_zipperator<It, Args...>> {
        using type = typename std::tuple_element<index, std::tuple<It, Args...>>::type;
    };

}

namespace aul {

    template<std::size_t i, class It0, class...Args>
    typename std::tuple_element<i, Forward_zipperator<It0, Args...>>::type
    get(const Forward_zipperator<It0, Args...>& zip) {
        return std::get<i>(zip.operator->());
    }

    template<std::size_t i, class It0, class...Args>
    typename std::tuple_element<i, Bidirectional_zipperator<It0, Args...>>::type
    get(const Bidirectional_zipperator<It0, Args...>& zip) {
        return std::get<i>(zip.operator->());
    }

    template<std::size_t i, class It0, class...Args>
    typename std::tuple_element<i, Random_access_zipperator<It0, Args...>>::type
    get(const Random_access_zipperator<It0, Args...>& zip) {
        return std::get<i>(zip.operator->());
    }

}

#endif //AUL_ZIPPER_ITERATOR_HPP

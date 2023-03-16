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
    class Forward_zipper_iterator {
    public:

        static_assert(
            sizeof...(Its),
            "Attempted to create a zipper_iterator that does not abstract over "
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

        using sub_iterator = Forward_zipper_iterator<Its...>;

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

        explicit Forward_zipper_iterator(It it, Its...its):
            it(it),
            its(its...) {}

        Forward_zipper_iterator() = default;
        Forward_zipper_iterator(const Forward_zipper_iterator&) = default;
        Forward_zipper_iterator(Forward_zipper_iterator&&) noexcept = default;
        ~Forward_zipper_iterator() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Forward_zipper_iterator& operator=(const Forward_zipper_iterator&) = default;
        Forward_zipper_iterator& operator=(Forward_zipper_iterator&&) noexcept = default;

        //=================================================
        // Comparison operators
        //=================================================

        bool operator==(const Forward_zipper_iterator& rhs) const {
            return (it == rhs.it0) && (its == rhs.its);
        }

        bool operator!=(const Forward_zipper_iterator& rhs) const {
            return (it != rhs.it0) || (its != rhs.its);
        }

        //=================================================
        // Increment operators
        //=================================================

        Forward_zipper_iterator& operator++() {
            ++it;
            ++its;
            return *this;
        }

        Forward_zipper_iterator operator++(int) {
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
        //operator Forward_zipper_iterator<const It0_2, Args2...>() const {//TODO: Implement}

    protected:

        //=================================================
        // Instance members
        //=================================================

        It it;
        sub_iterator its;

    };

    ///
    /// Specialization of Forward_zipper_iterator for a single iterator
    ///
    /// \tparam It Iterator type
    template<class It0, class It1>
    class Forward_zipper_iterator<It0, It1> {
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

        explicit Forward_zipper_iterator(It0 it0, It1 it1):
            it0(it0),
            it1(it1) {}

        Forward_zipper_iterator() = default;
        Forward_zipper_iterator(const Forward_zipper_iterator&) = default;
        Forward_zipper_iterator(Forward_zipper_iterator&&) noexcept = default;
        ~Forward_zipper_iterator() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Forward_zipper_iterator& operator=(const Forward_zipper_iterator&) = default;
        Forward_zipper_iterator& operator=(Forward_zipper_iterator&&) noexcept = default;

        //=================================================
        // Comparison operators
        //=================================================

        bool operator==(const Forward_zipper_iterator& rhs) const {
            return (it0 == rhs.it0) && (it1 == rhs.it1);
        }

        bool operator!=(const Forward_zipper_iterator& rhs) const {
            return (it0 != rhs.it0) || (it1 != rhs.it1);
        }

        //=================================================
        // Increment operators
        //=================================================

        Forward_zipper_iterator& operator++() {
            ++it0;
            ++it1;
            return *this;
        }

        Forward_zipper_iterator operator++(int) {
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
    class Bidirectional_zipper_iterator : public Bidirectional_zipper_iterator<Its...> {
        using base = Bidirectional_zipper_iterator<Its...>;
    public:

        static_assert(
            sizeof...(Its),
            "Attempted to create a zipper_iterator that does not abstract over "
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

        explicit Bidirectional_zipper_iterator(It it, Its...its):
            base(it),
            its(its...) {}

        Bidirectional_zipper_iterator() = default;
        Bidirectional_zipper_iterator(const Bidirectional_zipper_iterator&) = default;
        Bidirectional_zipper_iterator(Bidirectional_zipper_iterator&&) noexcept = default;
        ~Bidirectional_zipper_iterator() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Bidirectional_zipper_iterator& operator=(const Bidirectional_zipper_iterator&) = default;
        Bidirectional_zipper_iterator& operator=(Bidirectional_zipper_iterator&&) noexcept = default;

        //=================================================
        // Decrement_operators
        //=================================================

        Bidirectional_zipper_iterator& operator--() {
            --base::it0;
            return *this;
        }

        Bidirectional_zipper_iterator operator--(int) {
            auto tmp = *this;
            --base::it0;
            return tmp;
        }

    private:

        //=================================================
        // Instance members
        //=================================================

        Bidirectional_zipper_iterator<Its...> its;

    };

    template<class It0, class It1>
    class Bidirectional_zipper_iterator<It0, It1> : public Forward_zipper_iterator<It0, It1> {
        using base = Forward_zipper_iterator<It0, It1>;
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

        explicit Bidirectional_zipper_iterator(It0 it0, It1 it1):
            base(it0, it1) {}

        Bidirectional_zipper_iterator() = default;
        Bidirectional_zipper_iterator(const Bidirectional_zipper_iterator&) = default;
        Bidirectional_zipper_iterator(Bidirectional_zipper_iterator&&) noexcept = default;
        ~Bidirectional_zipper_iterator() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Bidirectional_zipper_iterator& operator=(const Bidirectional_zipper_iterator&) = default;
        Bidirectional_zipper_iterator& operator=(Bidirectional_zipper_iterator&&) = default;

        //=================================================
        // Decrement_operators
        //=================================================

        Bidirectional_zipper_iterator& operator--() {
            --base::it;
            return *this;
        }

        Bidirectional_zipper_iterator operator--(int) {
            auto tmp = *this;
            --base::it;
            return tmp;
        }

    };

    template<class It, class...Its>
    class Random_access_zipper_iterator : public Random_access_zipper_iterator<Its...> {
        using base = Random_access_zipper_iterator<Its...>;
    public:

        static_assert(
            sizeof...(Its),
            "Attempted to create a zipper_iterator that does not abstract over "
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

        explicit Random_access_zipper_iterator(It it, Its... its):
            base(its...),
            it(it) {}

        Random_access_zipper_iterator() = default;
        Random_access_zipper_iterator(const Random_access_zipper_iterator&) = default;
        Random_access_zipper_iterator(Random_access_zipper_iterator&&) noexcept = default;
        ~Random_access_zipper_iterator() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Random_access_zipper_iterator& operator=(const Random_access_zipper_iterator&) = default;
        Random_access_zipper_iterator& operator=(Random_access_zipper_iterator&&) noexcept = default;

        //=================================================
        // Comparison operators
        //=================================================

        bool operator<(const Random_access_zipper_iterator& rhs) {
            return (base::it < rhs.it);
        }

        bool operator>(const Random_access_zipper_iterator& rhs) {
            return (base::it > rhs.it);
        }

        bool operator>=(const Random_access_zipper_iterator& rhs) {
            return (base::it >= rhs.it);
        }

        bool operator<=(const Random_access_zipper_iterator& rhs) {
            return (base::it <= rhs.it);
        }

        //=================================================
        // Arithmetic assignment operators
        //=================================================

        Random_access_zipper_iterator& operator+=(difference_type d) {
            base::operator+=(d);
            it += d;
            return *this;
        }

        Random_access_zipper_iterator& operator-=(difference_type d) {
            base::operator-=(d);
            it -= d;
            return *this;
        }

        //=================================================
        // Arithmetic operators
        //=================================================

        Random_access_zipper_iterator operator+(difference_type rhs) const {
            auto ret = *this;
            ret.it += rhs;
            ret.base::operator+=(rhs);
            return ret;
        }

        friend Random_access_zipper_iterator operator+(difference_type lhs, Random_access_zipper_iterator rhs) {
            return (rhs + lhs);
        }

        Random_access_zipper_iterator operator-(difference_type rhs) const {
            auto ret = *this;
            ret.it0 -= rhs;
            ret.base::operator-=(rhs);
            return ret;
        }

        friend Random_access_zipper_iterator operator-(difference_type lhs, Random_access_zipper_iterator rhs) {
            return (rhs - lhs);
        }

        friend difference_type operator-(Random_access_zipper_iterator lhs, Random_access_zipper_iterator rhs) {
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
    class Random_access_zipper_iterator<It0, It1> : public Bidirectional_zipper_iterator<It0, It1> {
        using base = Bidirectional_zipper_iterator<It0, It1>;
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

        explicit Random_access_zipper_iterator(It0 it0, It1 it1):
            base(it0, it1) {}

        Random_access_zipper_iterator() = default;
        Random_access_zipper_iterator(const Random_access_zipper_iterator&) = default;
        Random_access_zipper_iterator(Random_access_zipper_iterator&&) noexcept = default;
        ~Random_access_zipper_iterator() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Random_access_zipper_iterator& operator=(const Random_access_zipper_iterator&) = default;
        Random_access_zipper_iterator& operator=(Random_access_zipper_iterator&&) noexcept = default;

        //=================================================
        // Comparison operators
        //=================================================

        bool operator<(const Random_access_zipper_iterator& rhs) const {
            return (base::it0 < rhs.it0);
        }

        bool operator>(const Random_access_zipper_iterator& rhs) const {
            return (base::it0 > rhs.it0);
        }

        bool operator>=(const Random_access_zipper_iterator& rhs) const {
            return (base::it0 >= rhs.it0);
        }

        bool operator<=(const Random_access_zipper_iterator& rhs) const {
            return (base::it0 <= rhs.it0);
        }

        //=================================================
        // Arithmetic assignment operators
        //=================================================

        Random_access_zipper_iterator& operator+=(difference_type d) {
            base::it0 += d;
            base::it1 += d;
            return *this;
        }

        Random_access_zipper_iterator& operator-=(difference_type d) {
            base::it0 -= d;
            base::it1 -= d;
            return *this;
        }

        //=================================================
        // Arithmetic operators
        //=================================================

        Random_access_zipper_iterator operator+(difference_type rhs) const {
            auto ret = *this;
            ret.it0 += rhs;
            ret.it1 += rhs;
            return ret;
        }

        friend Random_access_zipper_iterator operator+(difference_type lhs, Random_access_zipper_iterator rhs) {
            return (rhs + lhs);
        }

        Random_access_zipper_iterator operator-(difference_type rhs) const {
            auto ret = *this;
            ret.it0 -= rhs;
            ret.it1 -= rhs;
            return ret;
        }

        friend difference_type operator-(Random_access_zipper_iterator lhs, Random_access_zipper_iterator rhs) {
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
    struct tuple_size<aul::Forward_zipper_iterator<It, Args...>> {
        static constexpr std::size_t value = 1 + sizeof...(Args);
    };

    template<class It, class...Args>
    struct tuple_size<aul::Bidirectional_zipper_iterator<It, Args...>> {
        static constexpr std::size_t value = 1 + sizeof...(Args);
    };

    template<class It, class...Args>
    struct tuple_size<aul::Random_access_zipper_iterator<It, Args...>> {
        static constexpr std::size_t value = 1 + sizeof...(Args);
    };

    //=====================================================
    // std::tuple_element specializations
    //=====================================================

    template<std::size_t index, class It, class...Args>
    struct tuple_element<index, aul::Forward_zipper_iterator<It, Args...>> {
        using type = typename std::tuple_element<index, std::tuple<It, Args...>>::type;
    };

    template<std::size_t index, class It, class...Args>
    struct tuple_element<index, aul::Bidirectional_zipper_iterator<It, Args...>> {
        using type = typename std::tuple_element<index, std::tuple<It, Args...>>::type;
    };

    template<std::size_t index, class It, class...Args>
    struct tuple_element<index, aul::Random_access_zipper_iterator<It, Args...>> {
        using type = typename std::tuple_element<index, std::tuple<It, Args...>>::type;
    };

}

namespace aul {

    template<std::size_t i, class It0, class...Args>
    typename std::tuple_element<i, Forward_zipper_iterator<It0, Args...>>::type
    get(const Forward_zipper_iterator<It0, Args...>& zip) {
        return std::get<i>(zip.operator->());
    }

    template<std::size_t i, class It0, class...Args>
    typename std::tuple_element<i, Bidirectional_zipper_iterator<It0, Args...>>::type
    get(const Bidirectional_zipper_iterator<It0, Args...>& zip) {
        return std::get<i>(zip.operator->());
    }

    template<std::size_t i, class It0, class...Args>
    typename std::tuple_element<i, Random_access_zipper_iterator<It0, Args...>>::type
    get(const Random_access_zipper_iterator<It0, Args...>& zip) {
        return std::get<i>(zip.operator->());
    }

}

#endif //AUL_ZIPPER_ITERATOR_HPP

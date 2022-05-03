//
// Created by avereniect on 1/9/22.
//

#ifndef AUL_ZIPPER_ITERATOR_HPP
#define AUL_ZIPPER_ITERATOR_HPP

#include "../Utility.hpp"

#include <tuple>
#include <iterator>

namespace aul {

    template<class It, class...Its>
    class Forward_zipperator {
    public:

        //=================================================
        // Type aliases
        //=================================================

    private:

        using sub_iterator = Forward_zipperator<Its...>;

    public:

        using value_type = std::tuple<
            typename std::iterator_traits<It>::value_type,
            typename sub_iterator::value_type
        >;

        using difference_type = aul::widest_int_t<
            typename std::iterator_traits<It>::difference_type,
            typename sub_iterator::difference_type
        >;

        using pointer = std::tuple<
            typename std::iterator_traits<It>::pointer,
            typename std::iterator_traits<Its...>::pointer
        >;

        using reference = std::tuple<
            typename std::iterator_traits<It>::reference,
            typename std::iterator_traits<Its...>::reference
        >;

        using iterator_category = std::forward_iterator_tag;

        //=================================================
        // -ctors
        //=================================================

        explicit Forward_zipperator(It it0, Its...its):
            it0(it0),
            its(its...) {}

        Forward_zipperator() = default;
        Forward_zipperator(const Forward_zipperator&) = default;
        Forward_zipperator(Forward_zipperator&&) = default;
        ~Forward_zipperator() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Forward_zipperator& operator=(const Forward_zipperator&) = default;
        Forward_zipperator& operator=(Forward_zipperator&&) = default;

        //=================================================
        // Comparison operators
        //=================================================

        bool operator==(const Forward_zipperator& rhs) const {
            return (it0 == rhs.it0) && (its == rhs.its);
        }

        bool operator!=(const Forward_zipperator& rhs) const {
            return (it0 != rhs.it0) || (its != rhs.its);
        }

        //=================================================
        // Increment operators
        //=================================================

        Forward_zipperator& operator++() {
            ++it0;
            ++its;
            return *this;
        }

        Forward_zipperator operator++(int) {
            auto tmp = *this;
            ++it0;
            ++its;
            return tmp;
        }

        //=================================================
        // Dereference operators
        //=================================================

        reference operator*() const {
            if constexpr (sizeof...(Its) == 1) {
                return std::tie(*it0, *its);
            } else {
                return std::tuple_cat(std::tuple(it0), *its);
            }
        }

        pointer operator->() const {
            if constexpr (std::is_pointer_v<It>) {
                return std::tuple_cat(it0, its.operator->());
            } else {
                return std::tuple_cat(it0.operator->(), its.operator->());
            }
        }

    protected:

        It it0;
        sub_iterator its;

    };

    template<class It>
    class Forward_zipperator<It> {
    public:

        //=================================================
        // Type aliases
        //=================================================

        using value_type = typename std::iterator_traits<It>::value_type;
        using difference_type = typename std::iterator_traits<It>::difference_type;
        using pointer = typename std::iterator_traits<It>::pointer;
        using reference = typename std::iterator_traits<It>::reference;
        using iterator_category = std::forward_iterator_tag;

        //=================================================
        // -ctors
        //=================================================

        explicit Forward_zipperator(It it):
            it(it) {}

        Forward_zipperator() = default;
        Forward_zipperator(const Forward_zipperator&) = default;
        Forward_zipperator(Forward_zipperator&&) = default;
        ~Forward_zipperator() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Forward_zipperator& operator=(const Forward_zipperator&) = default;
        Forward_zipperator& operator=(Forward_zipperator&&) = default;

        //=================================================
        // Comparison operators
        //=================================================

        bool operator==(const Forward_zipperator& rhs) const {
            return (it == rhs.it);
        }

        bool operator!=(const Forward_zipperator& rhs) const {
            return (it != rhs.it);
        }

        //=================================================
        // Increment operators
        //=================================================

        Forward_zipperator& operator++() {
            ++it;
            return *this;
        }

        Forward_zipperator operator++(int) {
            auto tmp = *this;
            ++it;
            return tmp;
        }

        //=================================================
        // Dereference operators
        //=================================================

        reference operator*() const {
            return *it;
        }

        pointer operator->() const {
            if constexpr (std::is_pointer_v<It>) {
                return it;
            } else {
                return it.operator->();
            }
        }

    protected:

        It it;

    };

    template<class It, class...Its>
    class Bidirectional_zipperator : public Forward_zipperator<It, Its...> {
        using base = Forward_zipperator<It, Its...>;
    public:

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

        using base::base;

        Bidirectional_zipperator() = default;
        Bidirectional_zipperator(const Bidirectional_zipperator&) = default;
        Bidirectional_zipperator(Bidirectional_zipperator&&) = default;
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
            --base::it0;
            return *this;
        }

        Bidirectional_zipperator operator--(int) {
            auto tmp = *this;
            --base::it0;
            return tmp;
        }

    };

    template<class It>
    class Bidirectional_zipperator<It> : public Forward_zipperator<It> {
        using base = Forward_zipperator<It>;
    public:

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

        using base::base;

        Bidirectional_zipperator() = default;
        Bidirectional_zipperator(const Bidirectional_zipperator&) = default;
        Bidirectional_zipperator(Bidirectional_zipperator&&) = default;
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
    class Random_access_zipperator : public Bidirectional_zipperator<It, Its...> {
        using base = Bidirectional_zipperator<It, Its...>;
    public:

        //=================================================
        // Type aliases
        //=================================================

        using value_type = typename std::iterator_traits<It>::value_type;
        using difference_type = typename std::iterator_traits<It>::difference_type;
        using pointer = typename std::iterator_traits<It>::pointer;
        using reference = typename std::iterator_traits<It>::reference;
        using iterator_category = std::random_access_iterator_tag;

        //=================================================
        // -ctors
        //=================================================

        using base::base;

        Random_access_zipperator() = default;
        Random_access_zipperator(const Random_access_zipperator&) = default;
        Random_access_zipperator(Random_access_zipperator&&) = default;
        ~Random_access_zipperator() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Random_access_zipperator& operator=(const Random_access_zipperator&) = default;
        Random_access_zipperator& operator=(Random_access_zipperator&&) = default;

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
            base::it0 += d;
            base::its += d;
            return *this;
        }

        Random_access_zipperator& operator-=(difference_type d) {
            base::it0 -= d;
            base::its += d;
            return *this;
        }

        //=================================================
        // Arithmetic operators
        //=================================================

        Random_access_zipperator operator+(difference_type rhs) {
            auto ret = *this;
            ret.it0 += rhs;
            ret.its += rhs;
            return ret;
        }

        friend Random_access_zipperator operator+(difference_type lhs, Random_access_zipperator rhs) {
            return (rhs + lhs);
        }

        Random_access_zipperator operator-(difference_type rhs) {
            auto ret = *this;
            ret.it0 -= rhs;
            ret.its -= rhs;
            return ret;
        }

        friend Random_access_zipperator operator-(difference_type lhs, Random_access_zipperator rhs) {
            return (rhs - lhs);
        }

        //=================================================
        // Dereference operators
        //=================================================

        reference operator[](difference_type d) {
            if constexpr (sizeof...(Its)) {
                return std::tie(*base::it, base::its);
            } else {
                return std::tuple_cat(std::tuple(*base::it), base::its);
            }
        }

    };

    template<class It>
    class Random_access_zipperator<It> : public Bidirectional_zipperator<It> {
        using base = Bidirectional_zipperator<It>;
    public:

        //=================================================
        // Type aliases
        //=================================================

        using value_type = typename std::iterator_traits<It>::value_type;
        using difference_type = typename std::iterator_traits<It>::difference_type;
        using pointer = typename std::iterator_traits<It>::pointer;
        using reference = typename std::iterator_traits<It>::reference;
        using iterator_category = std::random_access_iterator_tag;

        //=================================================
        // -ctors
        //=================================================

        using base::base;

        Random_access_zipperator() = default;
        Random_access_zipperator(const Random_access_zipperator&) = default;
        Random_access_zipperator(Random_access_zipperator&&) = default;
        ~Random_access_zipperator() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Random_access_zipperator& operator=(const Random_access_zipperator&) = default;
        Random_access_zipperator& operator=(Random_access_zipperator&&) = default;

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
            base::it += d;
            return *this;
        }

        Random_access_zipperator& operator-=(difference_type d) {
            base::it -= d;
            return *this;
        }

        //=================================================
        // Arithmetic operators
        //=================================================

        Random_access_zipperator operator+(difference_type rhs) {
            auto ret = *this;
            ret.it += rhs;
            return ret;
        }

        friend Random_access_zipperator operator+(difference_type lhs, Random_access_zipperator rhs) {
            return (rhs + lhs);
        }

        Random_access_zipperator operator-(difference_type rhs) {
            auto ret = *this;
            ret.it -= rhs;
            return ret;
        }

        friend Random_access_zipperator operator-(difference_type lhs, Random_access_zipperator rhs) {
            return (rhs - lhs);
        }

        //=================================================
        // Dereference operators
        //=================================================

        reference operator[](difference_type d) {
            return *base::it;
        }

    };

}

#endif //AUL_ZIPPER_ITERATOR_HPP

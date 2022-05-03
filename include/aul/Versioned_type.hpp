#ifndef AUL_VERSIONED_TYPE_HPP
#define AUL_VERSIONED_TYPE_HPP

#include <type_traits>
#include <utility>

namespace aul {

    ///
    /// \tparam T Type to version
    /// \tparam V Integral type to use as version
    template<typename T, typename V = unsigned>
    class Versioned_type {
    public:

        static_assert(std::is_integral<V>::value);

        //=================================================
        // Type aliases
        //=================================================

        using value_type = T;
        using version_type = V;

        //=================================================
        // -ctors
        //=================================================

        Versioned_type(const T& x = T()) :
            dat(x),
            ver(V{}) {}

        Versioned_type(const T& x, const V v) :
            dat(x),
            ver(v) {}

        Versioned_type(const Versioned_type& r) :
            dat(r.dat),
            ver(r.ver) {}

        Versioned_type(Versioned_type&& r) noexcept :
            dat(std::move(r)),
            ver(r.ver) {

            r.ver = 0;
        }

        //=================================================
        // Assignment operators
        //=================================================

        Versioned_type& operator=(const Versioned_type& x) {
            this->dat = x.dat;
            this->ver = x.ver;

            return *this;
        }

        Versioned_type& operator=(Versioned_type&& x) {
            this->dat = std::move(x.dat);
            this->ver = x.ver;

            return *this;
        }

        Versioned_type& operator=(const T& t) {
            this->dat = t;
            ++ver;

            return *this;
        }

        Versioned_type& operator=(T&& t) {
            dat = std::move(t);
            ++ver;

            return *this;
        }

        //Conversion operator
        operator T() {
            return dat;
        }

        //=================================================
        //Access methods
        //=================================================

        T& data() {
            return dat;
        }

        const T& data() const {
            return dat;
        }

        version_type& version() {
            return ver;
        }

        version_type version() const {
            return ver;
        }

    private:

        //=================================================
        //Instance variables
        //=================================================

        T dat;
        version_type ver = 0;

    };

}

#endif

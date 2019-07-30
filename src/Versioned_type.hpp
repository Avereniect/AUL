#ifndef AUL_VERSIONED_TYPE_HPP
#define AUL_VERSIONED_TYPE_HPP

#include <type_traits>
#include <utility>

namespace aul {

	template<typename T, typename V = unsigned>
	class Versioned_type {
	public:
		using value_type = T;
		using pointer = T * ;
		using const_pointer = const T *;
		using reference = T & ;
		using const_reference = const T&;
		using difference_type = std::ptrdiff_t;
		using version_type = V;

		//constructor
		Versioned_type(const T& x = T()) :
			dat(x),
			ver(V())
		{}

		Versioned_type(const T& x, const V v) :
			dat(x),
			ver(v)
		{}

		//Copy constructor
		Versioned_type(const Versioned_type& r) :
			dat(r.dat),
			ver(r.ver)
		{}

		//Move constructor
		Versioned_type(Versioned_type&& r) noexcept :
			dat(std::move(r)),
			ver(r.ver)
		{
			r.ver = 0;
		}

		//Copy assignment
		Versioned_type& operator=(const Versioned_type& x) {
			this->dat = x.dat;
			this->ver = x.ver;

			return *this;
		}

		//Move assignment
		Versioned_type& operator=(Versioned_type&& x) {
			this->dat = std::move(x.dat);
			this->ver = x.ver;

			return *this;
		}

		//Type copy assignment
		Versioned_type& operator=(const T& t) {
			this->dat = t;
			++ver;

			return *this;
		}

		//Type move assignement
		Versioned_type& operator=(T&& t) {
			dat = std::move(t);
			++ver;

			return *this;
		}

		//Conversion operator
		operator T() {
			return dat;
		}

		//Access methods

		T& data() {
			return dat;
		}

		version_type& version() {
			return ver;
		}
		const_reference data() const {
			return dat;
		}

		version_type version() const {
			return ver;
		}

	private:
		T dat;
		version_type ver;

	};

	class Compact_versioned_int {
	public:


	private:


	};
}

#endif

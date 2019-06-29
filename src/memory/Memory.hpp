#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <memory>
#include <vector>

namespace aul {

	/*	Returns true if all bytes in object t share the same bit pattern.
	*/
	template<class T>
	constexpr inline bool has_uniform_bytes(const T& t) {
		if constexpr (sizeof(T) == 1) {
			return true;
		}

		const char* ptr = reinterpret_cast<const char*>(std::addressof(t));
		for (unsigned i = 0; i + 1 < sizeof(T); ++i) {
			if(*ptr != *(ptr + 1))
				return false;
		}
		return true;
	}

	/*	Destroys the objects in the range specified by [first, last) by calling
		std::allocaotr_traits<Alloc>::destroy() on the individual elements;
	*/
	template<class Forward_iter, class Alloc>
	void destroy(Forward_iter first, Forward_iter last, Alloc& alloc) {
		for (; first != last; ++first) {
			std::allocator_traits<Alloc>::destroy(alloc, std::addressof(*first));
		}
	}

	/*	Default constructs elements in the range [begin, end).
	*/
	template<class Forward_iter, class Alloc>
	void uninitialized_default_construct(Forward_iter begin, Forward_iter end, Alloc& alloc) {
		Forward_iter x = begin;
		try {
			for (; x != end; ++x) {
				std::allocator_traits<Alloc>::construct(alloc, std::addressof(*x));
			}

		} catch (...) {
			aul::destroy(begin, x, alloc);

			throw;
		}
	}

	template<class Forward_iter, class T, class Alloc>
	void uninitialized_fill(Forward_iter begin, Forward_iter end, const T& value, Alloc allocator) {
		Forward_iter x = begin;
		try {
			for (; x != end; ++x) {
				std::allocator_traits<Alloc>::construct(allocator, std::addressof(*x), value);
			}

		} catch (...) {
			aul::destroy(begin, x, allocator);

			throw;
		}
	}

	template<class Forward_iter, class Size, class T, class Alloc>
	void uninitialized_fill_n(Forward_iter begin, const Size n, const T& value, Alloc& allocator) {
		Forward_iter x = begin;

		try {
			for (Size i = 0; i != n; ++i, ++x) {
				std::allocator_traits<Alloc>::construct(allocator, std::addressof(*x), value);
			}
		} catch (...) {

			throw;
		}
	}

	template<typename Input_iter, typename Forward_iter, class Alloc>
	void uninitialized_move(Input_iter begin, Input_iter end, Forward_iter dest, Alloc& allocator) {
		Forward_iter x = dest;

		try {
			for (Forward_iter i = begin; i < end; ++i, ++x) {
				std::allocator_traits<Alloc>::construct(allocator, std::addressof(*x), std::move(*i));
			}

		} catch (...) {
			for (; begin <= x; ++begin) {
				std::allocator_traits<Alloc>::destroy(allocator, std::addressof(*begin));
			}

			throw;
		}
	}
	
	template<typename Input_iter, typename Forward_iter, class Alloc>
	void uninitialized_copy(Input_iter begin, Input_iter end, Forward_iter dest, Alloc allocator) {
		Forward_iter x = dest;

		try {
			for (Input_iter i = begin; i < end; ++i, ++x) {
				
				std::allocator_traits<Alloc>::construct(allocator, std::addressof(*x), std::move(*i));
			}

		} catch (...) {
			for (; begin != x; ++begin) {
				std::allocator_traits<Alloc>::destroy(allocator, std::addressof(*begin));
			}

			throw;
		}
	}

	template<typename Forward_iter, class T, class Alloc>
	void uninitialized_iota(Forward_iter begin, Forward_iter end, T val, Alloc& allocator) {
		Forward_iter x = begin;
		try {
			for (; x < end; ++x) {
				std::allocator_traits<Alloc>::construct(allocator, std::addressof(*x), val);
				++val;
			}
			
		} catch (...) {
			aul::destroy(begin, x, allocator);
			
			throw;
		}
	}

	template<class Alloc>
	class Allocator_has_traivial_types {
	public:
		using value = std::false_type;
	private:

	};

	//=====================================================
	// Utility templates
	//=====================================================

	/*	Allows for iterators for the same containers specialized for the same
		types and allocators that use identical type aliases to prevent code
		duplication.
	*/
	template<typename Alloc>
	class Allocator_types {
	public:
		using value_type = typename std::allocator_traits<Alloc>::value_type;
		using pointer = typename std::allocator_traits<Alloc>::pointer;
		using const_pointer = typename std::allocator_traits<Alloc>::const_pointer;
		using difference_type = typename std::allocator_traits<Alloc>::difference_type;
	};

	//TODO: Complete implementation
	//https://en.cppreference.com/w/cpp/iterator/reverse_iterator
	/*
	template <class Iter>
	class Reverse_random_access_iterator {
	private:
		Iter it;

	public:
		static_assert(
			std::is_same<std::random_access_iterator_tag, Iter::iterator_category>::value,
			"aul::Reverse_random_access_iterator must be instantiated with random access iterator type"
		);

		using value_type = Iter::value_type;
		using pointer = Iter::pointer;
		using reference = Iter::reference;
		using difference_type = Iter::difference_type;
		using iterator_category = Iter::iterator_category;

		constexpr Reverse_random_access_iterator() {}

		constexpr explicit Reverse_random_access_iterator(Iter iter) :
			it(iter)
		{}

		template<class U>
		constexpr Reverse_random_access_iterator(const Reverse_random_access_iterator<U>& other) {
			it = other;
		}

		//-------------------------------------------------
		// Assignment operators
		//-------------------------------------------------

		template<class U>
		constexpr Reverse_random_access_iterator& operator=(const Reverse_random_access_iterator& iter) {

		}

		//-------------------------------------------------
		// Increment/decrement operators
		//-------------------------------------------------

		//-------------------------------------------------
		// Arithmetic operators
		//-------------------------------------------------
	};
	*/

}

#endif

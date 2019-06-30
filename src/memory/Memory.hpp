#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <memory>
#include <vector>
#include <type_traits>

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
		std::allocator_traits<Alloc>::destroy() on the individual elements;
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

	///
	/// Template which is used to determine whether an allocator has default
	/// types for a given type T. In practice this typically means using raw
	/// pointers.
	///
	template<typename T, class Alloc>
	class Allocator_has_trivial_types {

	private:
		using alloc_traits = std::allocator_traits<T>;

		using value_type = typename alloc_traits::value_type;
		using default_value_type = T;

		using pointer = typename alloc_traits::pointer;
		using default_pointer = value_type*;

		using const_pointer = typename alloc_traits::const_pointer;
		using default_const_pointer = typename std::pointer_traits<pointer>::template rebind<const value_type>;

		using void_pointer = typename alloc_traits::void_pointer;
		using default_void_pointer = typename std::pointer_traits<pointer>::template rebind<void>;

		using const_void_pointer = typename alloc_traits::const_void_pointer;
		using default_const_void_pointer = typename std::pointer_traits<pointer>::template rebind<const void>;

		using difference_type = typename alloc_traits::difference_type;
		using default_difference_type = typename std::pointer_traits<pointer>::difference_type;

		using size_type = typename alloc_traits::size_type;
		using default_size_type = typename std::make_unsigned<difference_type>::type;

	public:

		//Result is equivalent to std::true_type or std::false_type
		using value = std::integral_constant<bool,
			std::is_same<value_type, default_value_type>::value &&
			std::is_same<pointer, default_pointer>::value &&
			std::is_same<const_pointer, default_const_pointer>::value && 
			std::is_same<void_pointer, default_void_pointer>::value &&
			std::is_same<difference_type, default_difference_type>::value &&
			std::is_same<size_type, default_size_type>::value
		>;
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

}

#endif

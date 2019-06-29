#ifndef AUL_VECTOR_HPP
#define AUL_VECTOR_HPP


#include "../memory/Memory.hpp"

#include <cstddef>
#include <iterator>
#include <memory>
#include <algorithm>
#include <iostream>
#include <type_traits>


namespace aul {

	/*	aul::Vector is a modified implementation of std::vector offering more
		control for the user. Vector supports the setting of a custom growth 
		factor and it allows the user to sepcify a size of the small buffer
		optimization
	 */
	template<typename T, int SB_size = 0, class Alloc = std::allocator<T>>
	class Vector {

		template<bool is_const>
		class Vector_iterator;

		class Vector_base;

	public:
		
		//-------------------------------------------------
		// Type aliases
		//-------------------------------------------------

		using value_type  = T;

		using reference = T&;
		using const_reference = const T&;

		using pointer = T*;
		using const_pointer = const T*;

		using difference_type = std::ptrdiff_t;
		using size_type = std::size_t;

		using iterator = Vector_iterator<false>;
		using const_iterator = Vector_iterator<true>;
		using reverse_iterator = typename std::reverse_iterator<iterator>;
		using const_reverse_iterator = typename std::reverse_iterator<const_iterator>;

		using allocator_type = Alloc;

		//-------------------------------------------------
		// Constructors/Destructor 
		//-------------------------------------------------

		Vector() noexcept(noexcept(allocator_type())) : 
			base(0, allocator_type())
		{}

		explicit Vector(const allocator_type& allocator) noexcept :
			base(0, allocator)
		{}
		
		Vector(const size_type n, const T& val, const allocator_type& allocator = {}) :
			base(n, allocator)
		{
			assign(n, val);
		}

		Vector(const size_type n, const allocator_type& allocator = {}) :
			base(n, allocator)
		{
			assign(n, T());
		}

		Vector(const Vector& vec) :
			base(vec.end - vec.begin, std::allocator_traits<allocator_type>::select_on_container_copy_construction(vec.base.allocator))
		{
			static_assert(std::is_copy_constructible<T>::value, "Type T is not copy constructible");

			aul::uninitialized_copy(vec.base.begin, vec.base.last, base.begin, base.allocator);
		}

		Vector(const Vector& r, const allocator_type& allocator) :
			base(r.end - r.begin, allocator)
		{
			static_assert(std::is_copy_constructible<T>::value, "Type T is not copy constructible");

			aul::uninitialized_copy(r.base.begin, r.base.last, base.begin, base.allocator);
		}

		Vector(Vector&& vec) :
			base(std::move(vec.base))
		{}

		Vector(Vector&& vec, allocator_type allocator_type) {
			if (vec.base.allocator == allocator_type) {
				base = Vector_base(std::move(vec.base));
			} else {
				aul::uninitialized_move(vec.base.begin, vec.base.end, base.allocator, base.allocator);
			}
		}

		Vector(const std::initializer_list<T>& list) {
			aul::uninitialized_copy(list.begin(), list.end(), base.begin, base.allocator);
		}

		~Vector() {
			clear();
		}

		//-------------------------------------------------
		// Assignment operators
		//-------------------------------------------------

		Vector& operator=(const Vector& vec) {
			static_assert(std::is_copy_constructible<T>::value, "Type T is not copy contructible");
			clear();

			//TODO: Implement
		}

		Vector& operator=(Vector&& vec) noexcept {
			//Tests for default constructed allocator_typeator's ability to manage memory
			if (
				std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value ||
				base.allocator == vec.base.allocator
			) {
				base = Vector_base(vec.base);
			} else {
				//Otherwise allocate new memory and move data there

				base = Vector_base(vec.end - vec.begin);
				aul::uninitialized_move(vec.base.begin, vec.base.end, base.begin, base.allocator);
			}
		}

		Vector& operator=(const std::initializer_list<T>& r) {
			static_assert(std::is_copy_constructible<T>::value, "Type T is not copy contructible");
			assign(r);
		}

		void assign(const size_type n, const T& val) {
			reserve(n);
			aul::uninitialized_fill(base.begin, base.end, val, base.allocator);
		}

		template<class Iter>
		void assign(Iter begin, Iter end) {
			reserve(end - begin);
			aul::uninitialized_copy(begin, end, base.begin, base.allocator_type);
		}

		void assign(const std::initializer_list<T>& list) {
			reserve(list.end() - list.begin());
			aul::uninitialized_copy(list.begin(), list.end(), base.begin, base.allocator);
		}

		inline const allocator_type& get_allocator_typeator() const {
			return base.allocator;
		}

		//---------------------------------------------------------------------
		//	Access methods
		//---------------------------------------------------------------------

		reference at(size_type pos) {
			if (size() <= pos) {
				throw std::out_of_range("Error attempt to access out of range");
			}

			return *(base.begin + pos);
		}

		const_reference at(size_type pos) const {
			if (size() <= pos) {
				throw std::out_of_range("Error attempt to access out of range");
			}

			return *(base.begin + pos);
		}

		reference operator[](size_type pos) {
			return *(base.begin + pos);
		}

		const_reference operator[](size_type pos) const {
			return *(base.begin + pos);
		}

		reference front() {
			return *(base.begin);
		}

		const_reference front() const {
			return *(base.begin);
		}

		reference back() {
			return *(base.last - 1);
		}

		const_reference back() const {
			return *(base.last - 1);
		}

		pointer data() {
			return base.begin;
		}

		const_pointer data() const {
			return base.begin;
		}

		//---------------------------------------------------------------------
		//	Capacity methods
		//---------------------------------------------------------------------
		
		[[nodiscard]] inline bool empty() const noexcept {
			return base.last - base.begin;
		}

		inline size_type size() const noexcept {
			return base.last - base.begin;
		}

		size_type max_size() const noexcept {
			size_type x = std::allocator_traits<allocator_type>::max_size(base.allocator);
			size_type y = std::numeric_limits<size_type>::max();

			return std::min(x, y);
		}

		void reserve(size_type n) {
			if (n > max_size()) {
				throw std::length_error("Vector has reached max_size()");
			}

			Vector_base new_base(n);
			aul::uninitialized_move(base.begin, base.end, new_base.begin, base.allocator);
			base = std::move(new_base);
		}

		inline size_type capacity() const noexcept {
			return base.end - base.begin;
		}

		void shrink_to_fit() {
			Vector_base new_base{size(), std::move(base.allocator)};
			aul::uninitialized_move(base.begin, base.end, new_base.begin, base.allocator);
			base = std::move(new_base);
		}

		//---------------------------------------------------------------------
		//	Mutator methods
		//---------------------------------------------------------------------

		template<class...Args>
		iterator emplace(const_iterator pos, Args...args) {
			if ( (pos.operator->() < base.begin) || (base.last <= pos.operator->()) ) {
				throw std::out_of_range("Attempting to contruct element at position indicated by invalid iterator.");
			}

			pointer p = const_cast<pointer>(pos.operator->());
			base.allocator_type.construct(base.end, std::move(*(base.end - 1)));
			std::move_backward(p, base.end - 1, base.end);
			
			++base.end;
			++base.last;

			return iterator(p);
		}

		template<class...Args>
		iterator emplace_back(const_iterator pos, Args... args) {
			return emplace(end(), std::forward<Args>(args)...);
		}

		iterator insert(const_iterator pos, const T& val) {
			return emplace(pos, std::move(val));
		}

		iterator insert(const_iterator pos, T&& value) {
			emplace(pos, std::move(value));
		}

		template<class Iter>
		iterator insert(const_iterator pos, Iter begin, Iter end) {
			if ((pos.operator->() < base.begin) || (base.last <= pos.operator->())) {
				throw std::out_of_range("Attempting to contruct element at position indicated by invalid iterator.");
			}

			pointer p = const_cast<pointer>(pos.operator->());

			grow(size() + (end - begin));

			//TODO: Insert and offset elements

			return iterator(p);
		}

		iterator insert(const_iterator pos, std::initializer_list<T>& list) {
			return insert(pos, list.begin(), list.end());
		}

		void push_back(const T& value) {
			//TODO
		}

		void push_back(T&& value) {
			//TODO
		}

		void clear() noexcept {
			aul::destroy(base.begin, base.last, base.allocator);

			base = Vector_base();
		}

		void swap(Vector& vec) noexcept {
			std::swap(base, vec.base);
		}

		//---------------------------------------------------------------------
		//	Comparison operators
		//---------------------------------------------------------------------

		friend bool operator==(const Vector& lhs, const Vector& rhs) noexcept {
			if (lhs.size() == rhs.size()) {
				return true;
			}

			for (size_type i = 0; i < lhs.size(); ++i) {
				if (lhs[i] != rhs[i]) {
					return false;
				}
			}

			return true;
		}

		friend bool operator!=(const Vector& lhs, const Vector& rhs) noexcept {
			return !operator==(lhs, rhs);
		}



		//---------------------------------------------------------------------
		//	Iterator methods
		//---------------------------------------------------------------------

		inline iterator begin() noexcept {
			return iterator(base.begin);
		}

		inline const_iterator begin() const noexcept {
			return const_iterator(base.begin);
		}

		inline const_iterator cbegin() const noexcept {
			return const_iterator(base.begin);
		}

		
		inline iterator end() noexcept {
			return iterator(base.last);
		}

		inline const_iterator end() const noexcept {
			return const_iterator(base.last);
		}

		inline const_iterator cend() const noexcept {
			return const_iterator(base.last);
		}


		inline reverse_iterator rbegin() noexcept {
			return reverse_iterator(base.last - 1);
		}

		inline const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator(base.last - 1);
		}

		inline const_reverse_iterator crbegin() const noexcept {
			return const_reverse_iterator(base.last - 1);
		}


		inline reverse_iterator rend() noexcept {
			return reverse_iterator(base.begin - 1);
		}

		inline const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator(base.begin - 1);
		}

		inline const_reverse_iterator crend() const noexcept {
			return const_reverse_iterator(base.begin - 1);
		}

	private:
		Vector_base base;

		float growth_factor = 2.0f;

		void grow(size_type n) {
			if (n < size()) {
				return;
			} 

			if(max_size() < n) {
				throw std::length_error("Vector cannot growth further");
			}

			Vector_base new_base{static_cast<size_type>(capacity() * growth_factor)};
			aul::uninitialized_move(base.begin, base.end, new_base.begin, base.allocator);

			base = std::move(new_base);
		}

		//=====================================================================
		//	Helper classes
		//=====================================================================

		/*	
		*/
		template<bool is_const = false>
		class Vector_iterator {
		public:

			//-----------------------------------------------------------------
			//	Type aliases
			//-----------------------------------------------------------------

			using value_type = T;
			using pointer = typename std::conditional<is_const, const T*, T*>::type;
			using reference = typename std::conditional<is_const, const T&, T&>::type;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::random_access_iterator_tag;

			Vector_iterator(pointer p) noexcept :
				p(p)
			{}

			//-----------------------------------------------------------------
			//	Increment/Decrement operators
			//-----------------------------------------------------------------

			Vector_iterator& operator++() noexcept {
				++p;

				return *this;
			}

			Vector_iterator& operator--() noexcept {
				--p;

				return *this;
			}

			Vector_iterator& operator++(int) noexcept {
				Vector_iterator temp = *this;
				++p;

				return temp;
			}

			Vector_iterator& operator--(int) noexcept {
				Vector_iterator temp = *this;
				--p;

				return temp;
			}

			//-----------------------------------------------------------------
			//	Assignment arithmetic operators
			//-----------------------------------------------------------------

			Vector_iterator& operator+=(difference_type x) noexcept {
				if (p) {
					p += x;
				}

				return *this;
			}

			Vector_iterator& operator-=(difference_type x) noexcept {
				if (p) {
					p -= x;
				}

				return *this;
			}

			//-----------------------------------------------------------------
			//	Arithmetic operators
			//-----------------------------------------------------------------

			friend Vector_iterator& operator+(const Vector_iterator it, const difference_type x) noexcept {
				return Vector_iterator(it.p + x);
			}

			friend Vector_iterator& operator+(const difference_type x, const Vector_iterator it) noexcept {
				return Vector_iterator(it.p + x);
			}

			friend Vector_iterator& operator-(const Vector_iterator it, const difference_type x) noexcept {
				return Vector_iterator(it.p - x);
			}

			friend Vector_iterator& operator-(const difference_type x, const Vector_iterator it) noexcept {
				return Vector_iterator(it.p - x);
			}

			//-----------------------------------------------------------------
			//	Comparison operators
			//-----------------------------------------------------------------

			friend bool operator==(const Vector_iterator l, const Vector_iterator r) noexcept {
				return l.p == r.p;
			}

			friend bool operator!=(const Vector_iterator l, const Vector_iterator r) noexcept {
				return l.p != r.p;
			}

			friend bool operator<(const Vector_iterator l, const Vector_iterator r) noexcept {
				return l.p < r.p;
			}

			friend bool operator>(const Vector_iterator l, const Vector_iterator r) noexcept {
				return l.p > r.p;
			}

			friend bool operator<=(const Vector_iterator l, const Vector_iterator r) noexcept {
				return l.p <= r.p;
			}

			friend bool operator>=(const Vector_iterator l, const Vector_iterator r) noexcept {
				return l.p >= r.p;
			}

			//-----------------------------------------------------------------
			//	Dereference operators
			//-----------------------------------------------------------------

			reference operator[](difference_type x) {
				return *(p + x);
			}

			reference operator*() {
				return *p;
			}

			pointer operator->() {
				return p;
			}

		private:
			pointer p = nullptr;

		}; //End class Vector_iterator<T>


		/*	
		*/
		class Vector_base {
		public:

			//-----------------------------------------------------------------
			//	Instance variables
			//-----------------------------------------------------------------
			pointer begin = small_buffer;
			pointer last = small_buffer;
			pointer end = small_buffer;

			allocator_type allocator;

			//-----------------------------------------------------------------
			//	Constructors/Destructor
			//-----------------------------------------------------------------

			Vector_base(size_type n = 0u, allocator_type allocator = {}) :
				allocator(allocator)
			{}

			Vector_base(const Vector_base& r) {
				allocator = std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value ? r.allocator : allocator_type();
				allocate(r.end - r.begin);
			}

			Vector_base(Vector_base&& r) noexcept :
				begin(r.begin),
				last(r.last),
				end(r.end)
			{
				begin = nullptr;
				last = nullptr;
				end = nullptr;
			}

			//-----------------------------------------------------------------
			//	Operators
			//-----------------------------------------------------------------

			Vector_base& operator=(const Vector_base& base) {
				if (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value) {
					allocator = base.allocator;
				}

				
			}

			Vector_base& operator=(Vector_base&& vec) noexcept {
				begin = vec.begin;
				last = vec.last;
				end = vec.end;

				vec.begin = nullptr;
				vec.last = nullptr;
				vec.end = nullptr;

				return *this;
			}

		private:

			void allocate(size_type n) {
				try {
					if(begin) {
						begin = allocator.allocate(n, begin);
					} else {
						begin = allocator.allocate(n);
					}
					last = begin;
					end = begin + n;

				} catch (...) {
					allocator.deallocate(begin, n);
					begin = nullptr;
					last = nullptr;
					end = nullptr;

					throw;
				}
			}

		};	//End class Vector_base<T>

	}; //End class template Vector<T>

}

#endif

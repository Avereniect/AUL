#ifndef LRU_CACHE_HPP
#define FRU_CACHE_HPP

#include "../memory/Memory.hpp"

#include <cstddef>
#include <memory>
#include <algorithm>
#include <type_traits>

namespace aul {

	template <class Alloc_types, bool is_const>
	class LRU_cache_iterator {
	public:
		using pointer = typename Alloc_types::pointer;

	private:
		pointer mem_begin = nullptr;
		pointer mem_final = nullptr;

		pointer current = nullptr;
	};

    template<class T, class Alloc = std::allocator<T>>
	class LRU_cache {

		class LRU_cache_base;

	public:

		//=====================================================================
		// Type aliases
		//=====================================================================

		using value_type = T;

		using reference = T & ;
		using const_reference = const T&;

		using iterator = LRU_cache_iterator<false>;
		using const_iterator = LRU_cache_iterator<true>;

		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		using difference_type = std::ptrdiff_t;
		using size_type = std::size_t;

		using allocator_type = Alloc;

		//=====================================================================
		// Constructors/Destructor
		//=====================================================================

		
		LRU_cache(const size_type n = 0) :
			base(n)
		{
			//TODO: Implement
		}

		LRU_cache(const LRU_cache& cache) :
			base(cache.base)
		{
            //TODO: Implement
        }

        LRU_cache(LRU_cache&& cache) :
			base{ std::move(cache.base) }
		{
            //TODO: Implement
        }

        ~LRU_cache() {
			clear();
        }

        //=====================================================================
        // Access methods
        //=====================================================================



        //=====================================================================
        // Access operators
        //=====================================================================

		reference operator[](const size_type x) {
			return base.data[x];
		}

		const_reference operator[](const size_type x) const {
			return base.data[x];
		}

        //=====================================================================
        // Assignment operators
        //=====================================================================

        LRU_cache& operator=(const LRU_cache&) {
            //TODO: Implement
        }

        LRU_cache& operator=(LRU_cache&&) {
            //TODO: Implement
        }

        //=====================================================================
        // Comparison operators
        //=====================================================================

        friend bool operator==(const LRU_cache& lhs, const LRU_cache& rhs) {
            //TODO: Implement
        }

        friend bool operator!=(const LRU_cache& lhs, const LRU_cache& rhs) {
            //TODO: Implement
        }

		/*
        //=====================================================================
        // Iterator methods
        //=====================================================================

        iterator begin() {
            //TODO: Implement
        }

        const_iterator begin() const {
            //TODO: Implement
        }

        const_iterator cbegin() const {
            //TODO: Implement
        }

        iterator end() {
            //TODO: Implement
        }

        const_iterator end() const {
            //TODO: Implement
        }

        const_iterator cend() const {
            //TODO: Implement
        }



        reverse_iterator rbegin() {
            //TODO: Implement
        }

        const_reverse_iterator rbegin() const {
            //TODO: Implement
        }

        const_reverse_iterator crbegin() const {
            //TODO: Implement
        }

        reverse_iterator rend() {
            //TODO: Implement
        }

        const_reverse_iterator rend() const {
            //TODO: Implement
        }

        const_reverse_iterator crend() const {
            //TODO: Implement
        }

        //=====================================================================
        // Misc. methods
        //=====================================================================

        LRU_cache& swap(const LRU_cache&) {
            //TODO: Implement
        }

        size_type size() const {
            //TODO: Implement
        }

        size_type max_size() const {
            //TODO: Implement
        }

        bool empty() const {
            //TODO: Implement
        }

		allocator_type get_allocator() {
			return base.element_allocator;
		}

		*/

		void clear() noexcept(noexcept(T())) {

		}
        
    private:

		//=====================================================================
		// Instance members
		//=====================================================================

		LRU_cache_base base;

		//=====================================================================
		// Helper classes
		//=====================================================================

        class LRU_cache_base {
			using alloc_type = Alloc;
			using alloc_traits = std::allocator_traits<Alloc>;

        public:

			//=====================================================================
			// Instance members
			//=====================================================================

            T* data_begin = nullptr;
			T* data_end = nullptr;

			size_type size;

			T* front = 0;
			T* back  = 0;

			alloc_type allocator;

			//=====================================================================
			// Consructors/Destructor
			//=====================================================================

			LRU_cache_base(const size_type n) {
				this->allocate(n);
			}

			LRU_cache_base(const LRU_cache_base& cache_base) :
				capacity(cache_base.capacity),
				count(cache_base.count),
				allocator(alloc_traits::select_on_container_copy_construction(cache_base.data))
			{
				allocate(cache_base.capacity);
			}

			LRU_cache_base(LRU_cache_base&& cache_base) :
				versions(cache_base.versions),
				elements(cache_base.elements),
				capacity(cache_base.capacity),
				count(cache_base.count),
				element_allocator(std::move(cache_base.element_allocator)),
				version_allocator(std::move(cache_base.version_allocator))
			{
				cache_base.versions = nullptr;
				cache_base.elements = nullptr;
				cache_base.capacity = 0;
				cache_base.count = 0;
			}

			~LRU_cache_base() {
				deallocate();
			}

			//=====================================================================
			// Assignment operators
			//=====================================================================

			LRU_cache_base& operator=(const LRU_cache_base& cache_base) {
				deallocate();

				if (element_alloc_traits::propagate_on_container_copy_assignment::value) {
					element_allocator = cache_base.element_allocator;
				}

				if (version_alloc_traits::propagate_on_container_copy_assignment::value) {
					version_allocator = cache_base.version_allocator;
				}

				allocate(cache_base.count);

				return *this;
			}

			LRU_cache_base& operator=(LRU_cache_base&& cache_base) {
				deallocate();

				if (element_alloc_traits::propagate_on_container_move_assignment::value) {
					element_allocator = std::move(cache_base.element_allocator);
				}

				if (version_alloc_traits::propagate_on_container_move_assignment::value) {
					version_allocator = std::move(cache_base.version_allocator);
				}

				versions = cache_base.versions;
				elements = cache_base.elements;
				capacity = cache_base.capacity;
				count = cache_base.count;
				
				cache_base.versions = nullptr;
				cache_base.elements = nullptr;
				cache_base.capacity = 0;
				cache_base.count = 0;

				return *this;
			}

			LRU_cache_base& swap(LRU_cache_base& cache_base) {
				if (element_alloc_traits::propagate_on_container_swap::value) {
					std::swap(element_allocator, cache_base.element_allocator);
				}

				if (version_alloc_traits::propagate_on_container_swap::value) {
					std::swap(version_allocator, cache_base.version_allocator);
				}

				std::swap(elements, cache_base.elements);
				std::swap(versions, cache_base.versions);
				std::swap(capacity, cache_base.capacity);
				std::swap(count, cache_base.count);

				return *this;
			}

        private:

			void allocate(const size_type n) {

				try {
					data_begin = alloc_traits::allocate(allocator, n);

					capacity = n;

				} catch (...) {
					elements = nullptr;
					versions = nullptr;

					count = 0;
					capacity = 0;

					throw;
				}

			}

			void deallocate() {
				try {
					element_alloc_traits::deallocate(element_allocator, elements, count);
					version_alloc_traits::deallocate(version_allocator, versions, count);

					capacity = 0;
					count = 0;
				} catch (...) {

					throw;
				}
			}

        };

    };

	/*

	template <bool is_const>
	class LRU_cache_iterator {
	public:

		//=================================================================
		// Type aliases
		//=================================================================

		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using reference = T&;
		using pointer = T*;
		using iterator_category = std::random_access_iterator_tag;



		//=================================================================
		// Constructors/Destructor
		//=================================================================

		LRU_cache_iterator() {
			//TODO: Implement
		}

		LRU_cache_iterator(LRU_cache_iterator&&) {
			//TODO: Implement
		}

		LRU_cache_iterator(const LRU_cache_iterator&) {
			//TODO: Implement
		}

		~LRU_cache_iterator() {
			//TODO: Implement
		}

		//=================================================================
		// Assignment operators
		//=================================================================

		LRU_cache_iterator& operator=(LRU_cache_iterator&&) {
			//TODO: Implement
		}

		LRU_cache_iterator& operator=(const LRU_cache_iterator) {
			//TODO: Implement
		}

		LRU_cache_iterator& operator+=(difference_type offset) {
			//TODO: Implement
		}

		LRU_cache_iterator& operator-=(difference_type offset) {
			//TODO: Implement
		}

		//=================================================================
		// In/Decrement operators
		//=================================================================

		LRU_cache_iterator& operator++() {
			//TODO: Implement
		}

		LRU_cache_iterator& operator++(int) {
			//TODO: Implement
		}

		LRU_cache_iterator& operator--() {
			//TODO: Implement
		}

		LRU_cache_iterator& operator--(int) {
			//TODO: Implement
		}

		//=================================================================
		// Arithmetic operators
		//=================================================================

		friend LRU_cache_iterator operator+(const difference_type lhs, const LRU_cache_iterator rhs) {
			//TODO: Implement
		}

		friend LRU_cache_iterator operator+(const LRU_cache_iterator lhs, const difference_type rhs) {
			//TODO: Implement
		}

		friend LRU_cache_iterator operator-(const LRU_cache_iterator lhs, const difference_type rhs) {
			//TODO: Implement
		}

		difference_type operator-(const LRU_cache_iterator) {
			//TODO: Implement
		}

		//=================================================================
		// Comparison operators
		//=================================================================

		bool operator==(const LRU_cache_iterator) {
			//TODO: Implement
		}

		bool operator!=(const LRU_cache_iterator) {
			//TODO: Implement
		}

		bool operator<(const LRU_cache_iterator) {
			//TODO: Implement
		}

		bool operator>(const LRU_cache_iterator) {
			//TODO: Implement
		}

		bool operator<=(const LRU_cache_iterator) {
			//TODO: Implement
		}

		bool operator>=(const LRU_cache_iterator) {
			//TODO: Implement
		}

		//=================================================================
		// Dereference operators
		//=================================================================

		reference operator*() {
			//TODO: Implement
		}

		pointer operator->() {
			//TODO: Implement
		}

		reference operator[](const difference_type) {
			//TODO: Implement
		}

	private:
		pointer p = nullptr;
	};
	*/

}

namespace std {

	template<class T>
	void swap(aul::LRU_cache<T>& lhs, aul::LRU_cache<T>& rhs) {
		lhs.swap(rhs);
	}

}

#endif
#ifndef LINEAR_ALLOCATOR
#define LINEAR_ALLOCATOR

#include <type_traits>
#include <new>

namespace aul {

	namespace {

		//=================================================
		// Helper classes
		//=================================================

		class Linear_allocator_base {
		public:

			//=============================================
			// Instance variables
			//=============================================

			std::byte* begin = nullptr;
			std::byte* free = nullptr;
			std::byte* end = nullptr;

			std::size_t user_count = 1;

			//=============================================
			// Constructors
			//=============================================

			Linear_allocator_base(std::byte* begin, std::byte* end) :
				begin(begin),
				free(begin),
				end(end)
			{}

			Linear_allocator_base(Linear_allocator_base&& base) noexcept :
				begin(base.begin),
				free(base.free),
				end(base.end)
			{
				base.begin = base.free = base.end = nullptr;
			}

			Linear_allocator_base& operator=(Linear_allocator_base&& base) {
				begin = base.begin;
				free = base.free;
				end = base.end;

				base.begin = nullptr;
				base.free = nullptr;
				base.end = nullptr;
			}

		};

	}




	template <class T>
	class Linear_allocator {

		template<class U>
		friend class Linear_allocator;

	public:

		//=================================================
		// Type aliases 
		//=================================================

		using value_type = T;
		
		using propagate_on_container_copy_assignment = std::true_type;
		using propagate_on_container_move_assignment = std::true_type;
		using propagate_on_container_swap            = std::true_type;

		using is_always_equal = std::false_type;

		template <class U> struct rebind { typedef Linear_allocator<U> other; };

		//=================================================
		// Constructors
		//=================================================

		Linear_allocator() noexcept :
			base(nullptr)
		{}

		Linear_allocator(void* begin, void* end) noexcept :
			base(new Linear_allocator_base(reinterpret_cast<std::byte*>(begin), reinterpret_cast<std::byte*>(end)))
		{}

		Linear_allocator(void* begin, std::size_t n) noexcept :
			Linear_allocator(reinterpret_cast<std::byte*>(begin), reinterpret_cast<std::byte*>(begin) + n)
		{}
		
		template <class U>
		Linear_allocator(Linear_allocator<U> const& alloc) noexcept :
			base(alloc.base)
		{}

		Linear_allocator(const Linear_allocator& alloc) noexcept :
			base(alloc.base)
		{}

		Linear_allocator(Linear_allocator&& alloc) noexcept :
			base(std::move(alloc.base))
		{}

		//=================================================
		// Allocation/Deallocation methods
		//=================================================

		value_type* allocate(std::size_t n, void* p) {
			std::size_t byte_count = n * sizeof(T);

			if ((base->end - base->free) < byte_count) {
				throw std::bad_alloc();
			}

			value_type* block = reinterpret_cast<value_type*>(base->begin);

			base->begin += n * sizeof(T);

			return block;
		}

		void deallocate(value_type* p, std::size_t n) noexcept {
			//Do nothing
			(void)p;
			(void)n;
		}

		//=================================================
		// Allocation/Deallocation methods
		//=================================================

		void assign(void* p, std::size_t n) {

		}

		void reset() {
			base->free = base->begin;
		}

		std::byte* pool() {
			return base->begin;
		}

	private:

		//=================================================
		// Instance members
		//=================================================

		Linear_allocator_base* base = nullptr;
		
	};


}

#endif

#pragma once
#include "utility/memory.h"

namespace utility {
	class block_allocator {
		struct block {
			block(u8* memory);
			~block();

			u8* memory;
			u64 position;
			block* next = nullptr;
		};
	public:
		/**
		 * \brief Constructs a new block allocator.
		 * \param block_size Size of individual blocks [bytes]
		 */
		block_allocator(u64 block_size);
		~block_allocator();

		block_allocator(const block_allocator& other) = delete;
		block_allocator(block_allocator&& other) = delete;

		block_allocator& operator=(const block_allocator& other) = delete;
		block_allocator& operator=(block_allocator&& other) = delete;

		/**
		 * \brief Prints owned blocks and their contents.
		 */
		void print_bytes() const;

		/**
		 * \brief Prints % of used memory for every block.
		 */
		void print_used() const;

		/**
		 * \brief Allocates \b size bytes of uninitialized memory.
		 * \param size Amount of memory to allocate [bytes]
		 * \return Pointer to the beginning of the allocated region.
		 */
		auto allocate(u64 size) -> void*;

		/**
		 * \brief Allocates \b size bytes of zero-initialized memory.
		 * \param size Amount of memory to allocate [bytes]
		 * \return Pointer to the beginning of the allocated region.
		 */
		auto allocate_zero(u64 size) -> void*;

		/**
		 * \brief Allocates sizeof(type) bytes of uninitialized memory.
		 * \tparam type Type to allocate memory for
		 * \return Pointer to the allocated object.
		 */
		template<typename type>
		auto allocate() -> type* {
			return static_cast<type*>(allocate(sizeof(type)));
		}

		/**
		 * \brief Allocates sizeof(type) bytes of zero-initialized memory.
		 * \tparam type Type to allocate memory for
		 * \return Pointer to the allocated object.
		 */
		template<typename type>
		auto allocate_zero() -> type* {
			return static_cast<type*>(allocate_zero(sizeof(type)));
		}

		/**
		 * \brief Allocates sizeof(type) bytes of uninitialized memory and constructs \b type.
		 * \tparam type Type of the object to allocate and construct.
		 * \return Pointer to the constructed object.
		 */
		template<typename type, typename... value_types>
		auto emplace(value_types&&... values) -> type* {
			return new (allocate(sizeof(type))) type(std::forward<value_types>(values)...);
		}

		/**
		 * \brief Allocates sizeof(type) bytes of zero-initialized memory and constructs \b type.
		 * \tparam type Type of the object to allocate and construct.
		 * \return Pointer to the constructed object.
		 */
		template<typename type, typename... value_types>
		auto emplace_zero(value_types&&... values) -> type* {
			return new (allocate_zero(sizeof(type))) type(std::forward<value_types>(values)...);
		}

		/**
		 * \brief Retrieves the current amount of allocated blocks.
		 * \return Count of currently allocated blocks.
		 */
		auto get_block_count() const->u64;

		/**
		 * \brief Retrieves the max size of individual blocks [bytes].
		 * \return Max size of individual blocks [bytes].
		 */
		auto get_block_size() const -> u64;
	private:
		/**
		 * \brief Helper function for allocating new memory blocks
		 */
		void allocate_block();
	private:
		block* m_first_block = nullptr;
		block* m_current_block = nullptr;

		u64 m_block_size;
		u64 m_block_count = 0;
	};
}

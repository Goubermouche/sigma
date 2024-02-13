#pragma once
#include "macros.h"
#include "memory.h"

namespace utility {
	class block_allocator {
		struct block {
			block(u8* memory) : memory(memory), position(0) {}
			~block() {
				std::free(memory);
			}

			u8* memory;
			u64 position;
			block* next = nullptr;
		};
	public:
		/**
		 * \brief Constructs a new block allocator.
		 * \param block_size Size of individual blocks [bytes]
		 */
		block_allocator(u64 block_size) : m_block_size(block_size) {
			allocate_block(); // allocate the first block
			m_first_block = m_current_block;
		}

		~block_allocator() {
			while (m_first_block) {
				const block* temp = m_first_block;
				m_first_block = m_first_block->next;
				delete temp;
			}
		}

		block_allocator(const block_allocator& other) = delete;
		block_allocator(block_allocator&& other) = delete;

		block_allocator& operator=(const block_allocator& other) = delete;
		block_allocator& operator=(block_allocator&& other) = delete;

		/**
		 * \brief Prints owned blocks and their contents.
		 */
		void print_bytes() const {
			const u8 digit_count = num_digits(get_block_count()) + 3;
			const block* temp = m_first_block;
			u64 index = 0;

			while (temp) {
				console::print("{:<{}}", std::format("{}:", index++), digit_count);

				for (u64 i = 0; i < temp->position; i++) {
					console::print("{:02X} ", temp->memory[i]);
				}

				console::print("\n");
				temp = temp->next;
			}
		}

		/**
		 * \brief Prints % of used memory for every block.
		 */
		void print_used() const {
			const u8 digit_count = num_digits(get_block_count()) + 3;
			const double percent = static_cast<double>(m_block_size) / 100.0;
			const block* temp = m_first_block;

			u64 wasted_bytes = 0;
			u64 index = 0;

			while (temp) {
				double used_percent = static_cast<double>(temp->position) / percent;
				wasted_bytes += m_block_size - temp->position;
				temp = temp->next;

				console::print("{:<{}}{:.2f}%\n", std::format("{}:", index++), digit_count, used_percent);
			}

			const u64 total_bytes = get_block_count() * m_block_size;
			const double total_percent = static_cast<double>(total_bytes) / 100.0;
			const double wasted_percent = static_cast<double>(wasted_bytes) / total_percent;

			console::print("\nwasted {}B / {}B ({:.2f}%)\n", wasted_bytes, total_bytes, wasted_percent);
		}

		/**
		 * \brief Allocates \b size bytes of uninitialized memory.
		 * \param size Amount of memory to allocate [bytes]
		 * \return Pointer to the beginning of the allocated region.
		 */
		auto allocate(u64 size) -> void* {
			ASSERT(size <= m_block_size, "insufficient block size for allocation of {}B", size);

			if (size == 0) {
				return nullptr;
			}

			// if this allocation incurs a buffer overflow allocate a new block
			if (m_current_block->position + size >= m_block_size) {
				allocate_block();
			}

			void* memory = m_current_block->memory + m_current_block->position;
			m_current_block->position += size;
			return memory;
		}

		/**
		 * \brief Allocates \b size bytes of zero-initialized memory.
		 * \param size Amount of memory to allocate [bytes]
		 * \return Pointer to the beginning of the allocated region.
		 */
		auto allocate_zero(u64 size) -> void* {
			void* memory = allocate(size);
			std::memset(memory, 0, size);
			return memory;
		}

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
		auto get_block_count() const -> u64 {
			return m_block_count;
		}

		/**
		 * \brief Retrieves the max size of individual blocks [bytes].
		 * \return Max size of individual blocks [bytes].
		 */
		auto get_block_size() const -> u64 {
			return m_block_size;
		}
	private:
		/**
		 * \brief Helper function for allocating new memory blocks
		 */
		void allocate_block() {
			const auto memory = static_cast<u8*>(std::malloc(m_block_size));
			const auto new_block = new block(memory);

			if (m_current_block) {
				m_current_block->next = new_block;
			}

			m_current_block = new_block;
			m_block_count++;
		}
	private:
		block* m_first_block = nullptr;
		block* m_current_block = nullptr;

		u64 m_block_size;
		u64 m_block_count = 0;
	};
} // namespace utility

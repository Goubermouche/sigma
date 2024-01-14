#include "block_allocator.h"
#include "utility/macros.h"

namespace utility {
	block_allocator::block::block(u8* memory) : memory(memory), position(0) {}

	block_allocator::block::~block() {
		std::free(memory);
	}

	block_allocator::block_allocator(u64 block_size) : m_block_size(block_size) {
		allocate_block(); // allocate the first block
		m_first_block = m_current_block;
	}

	block_allocator::~block_allocator() {
		while (m_first_block) {
			const block* temp = m_first_block;
			m_first_block = m_first_block->next;
			delete temp;
		}
	}

	void block_allocator::print_bytes() const {
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

	void block_allocator::print_used() const {
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

	auto block_allocator::allocate(u64 size) -> void* {
		ASSERT(size <= m_block_size, "insufficient block size for allocation of {}B", size);

		// if this allocation incurs a buffer overflow allocate a new block
		if (m_current_block->position + size > m_block_size) {
			allocate_block();
		}

		void* memory = m_current_block->memory + m_current_block->position;
		m_current_block->position += size;
		return memory;
	}

	auto block_allocator::allocate_zero(u64 size) -> void* {
		void* memory = allocate(size);
		std::memset(memory, 0, size);
		return memory;
	}

	auto block_allocator::get_block_count() const -> u64 {
		return m_block_count;
	}

	auto block_allocator::get_block_size() const -> u64 {
		return m_block_size;
	}

	void block_allocator::allocate_block() {
		const auto memory = static_cast<u8*>(std::malloc(m_block_size));
		const auto new_block = new block(memory);

		if (m_current_block) {
			m_current_block->next = new_block;
		}

		m_current_block = new_block;
		m_block_count++;
	}
}

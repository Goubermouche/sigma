#include "block_allocator.h"
#include "utility/macros.h"

namespace utility {
	block_allocator::block::block(u8* memory) : memory(memory), position(0) {}

	block_allocator::block::~block() {
		std::free(memory);
	}

	block_allocator::block_allocator(u64 block_size) : m_block_size(block_size) {
		allocate_block(); // allocate the first block
	}

	block_allocator::block_allocator(block_allocator&& other) noexcept
		: m_blocks(std::move(other.m_blocks)), m_block_size(other.m_block_size) {}

	block_allocator& block_allocator::operator=(block_allocator&& other) noexcept {
		if (this != &other) {
			m_blocks = std::move(other.m_blocks);
			m_block_size = other.m_block_size;
		}

		return *this;
	}

	void block_allocator::print_bytes() const {
		const u8 digit_count = num_digits(m_blocks.size()) + 3;
		u64 index = 0;

		for (const auto& block : m_blocks) {
			std::cout << std::format("{:<{}}", std::format("{}:", index++), digit_count);

			for (u64 i = 0; i < block.position; i++) {
				std::cout << std::format("{:02X} ", block.memory[i]);
			}

			std::cout << '\n';
		}
	}

	void block_allocator::print_used() const {
		const u8 digit_count = num_digits(m_blocks.size()) + 3;
		const double percent = static_cast<double>(m_block_size) / 100.0;

		u64 wasted_bytes = 0;
		u64 index = 0;

		for (const auto& block : m_blocks) {
			double used_percent = static_cast<double>(block.position) / percent;
			wasted_bytes += m_block_size - block.position;
			std::cout << std::format("{:<{}}{:.2f}%\n", std::format("{}:", index++), digit_count, used_percent);
		}

		const u64 total_bytes = m_blocks.size() * m_block_size;
		const double total_percent = static_cast<double>(total_bytes) / 100.0;
		const double wasted_percent = static_cast<double>(wasted_bytes) / total_percent;

		std::cout << std::format("\nwasted {}B / {}B ({:.2f}%)\n", wasted_bytes, total_bytes, wasted_percent);
	}

	auto block_allocator::allocate(u64 size) -> void* {
		ASSERT(size <= m_block_size, "insufficient block size for allocation of {}B", size);

		// if this allocation incurs a buffer overflow allocate a new block
		if (m_blocks.back().position + size > m_block_size) {
			allocate_block();
		}

		auto& block = m_blocks.back();
		void* memory = block.memory + block.position;
		block.position += size;
		return memory;
	}

	auto block_allocator::allocate_zero(u64 size) -> void* {
		void* memory = allocate(size);
		std::memset(memory, 0, size);
		return memory;
	}

	auto block_allocator::get_block_count() const -> u64 {
		return m_blocks.size();
	}

	auto block_allocator::get_block_size() const -> u64 {
		return m_block_size;
	}

	void block_allocator::allocate_block() {
		m_blocks.emplace_back(static_cast<u8*>(std::malloc(m_block_size)));
	}
}

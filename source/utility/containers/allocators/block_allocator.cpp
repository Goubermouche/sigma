#include "block_allocator.h"

namespace utility {
	block_allocator::block_allocator(u64 block_size)
		: m_block_size(block_size), m_offset(0) {
		m_blocks.emplace_back(std::make_unique<u8[]>(m_block_size));
	}

	block_allocator::block_allocator(block_allocator&& other) noexcept
		: m_blocks(std::move(other.m_blocks)), m_block_size(other.m_block_size), m_offset(other.m_offset) {
		other.m_block_size = 0;
		other.m_offset = 0;
	}

	block_allocator& block_allocator::operator=(
		block_allocator&& other
	) noexcept {
		if (this != &other) {
			m_blocks = std::move(other.m_blocks);
			m_block_size = other.m_block_size;
			m_offset = other.m_offset;
			other.m_block_size = 0;
			other.m_offset = 0;
		}
		return *this;
	}

	void* block_allocator::allocate(u64 allocation_size) {
		if (m_block_size - m_offset < allocation_size) {
			m_blocks.emplace_back(std::make_unique<u8[]>(m_block_size));
			m_offset = 0;
		}

		void* ptr = m_blocks.back().get() + m_offset;
		m_offset += allocation_size;

		return ptr;
	}
}

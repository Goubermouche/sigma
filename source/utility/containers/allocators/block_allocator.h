#pragma once
#include "utility/memory.h"

namespace utility {
	class block_allocator {
	public:
		block_allocator() = default;
		block_allocator(u64 block_size);

		block_allocator(const block_allocator&) = delete;
		block_allocator(block_allocator&& other) noexcept;

		block_allocator& operator=(const block_allocator&) = delete;
		block_allocator& operator=(block_allocator&& other) noexcept;

		void* allocate(u64 allocation_size);
	private:
		std::list<u_ptr<u8[]>> m_blocks;
		u64 m_block_size;
		u64 m_offset;
	};
}

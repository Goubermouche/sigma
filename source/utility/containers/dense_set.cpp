#include "dense_set.h"

namespace utility {
	dense_set::dense_set(u64 capacity)
		: contiguous_container(zero_initialize((capacity + 63) / 64)), m_used_capacity(capacity) {}

	void dense_set::clear() {
		zero_fill();
	}

	auto dense_set::set_union(const dense_set& src) -> bool {
		ASSERT(m_used_capacity >= src.m_used_capacity, "panic");
		u64 n = (src.m_used_capacity + 63) / 64;
		u64 changes = 0;

		for (u64 i = 0; i < n; ++i) {
			u64 old = m_data[i];
			u64 new_val = old | src.m_data[i];

			m_data[i] = new_val;
			changes |= (old ^ new_val);
		}

		return changes;
	}

	void dense_set::copy(dense_set& src) {
		ASSERT(m_used_capacity >= src.m_used_capacity, "panic");
		copy_range(src.begin(), src.end(), begin());
	}

	void dense_set::put(u64 index)
	{
		u64 slot = index / 64;
		u64 pos = index % 64;

		if (slot >= m_size) {
			resize((index * 2 + 63) / 64, 0);
			m_used_capacity = index * 2;
		}

		m_data[slot] |= (1ull << pos);
	}

	void dense_set::remove(u64 index) const {
		u64 slot = index / 64;
		u64 pos = index % 64;

		if (slot < m_size) {
			m_data[slot] &= ~(1ull << pos);
		}
	}

	auto dense_set::get(u64 index) const -> bool {
		u64 slot = index / 64;
		u64 pos = index % 64;

		if (slot >= m_size) {
			return false;
		}

		return m_data[slot] & (1ull << pos);
	}

	auto dense_set::data(u64 index) const -> u64 {
		return m_data[index];
	}

	auto dense_set::data(u64 index) -> u64& {
		return m_data[index];
	}

	auto dense_set::capacity() const -> u64 {
		return m_used_capacity;
	}
}

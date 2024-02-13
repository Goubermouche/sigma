#pragma once
#include "../macros.h"

namespace utility {
	// TODO: add documentation
	// TODO: explore alternate data structures which may result
	//       in better performance/cleaner code

	class dense_set {
	public:
		dense_set() = default;
		dense_set(u64 capacity) : m_capacity(capacity) {
			m_data.resize((capacity + 63) / 64, 0);
			std::ranges::fill(m_data.begin(), m_data.end(), 0);
		}

		void clear() {
			std::fill(m_data.begin(), m_data.end(), 0);
		}

		bool set_union(const dense_set& src) {
			ASSERT(m_capacity >= src.m_capacity, "panic");
			u64 n = (src.m_capacity + 63) / 64;
			u64 changes = 0;

			for (u64 i = 0; i < n; ++i) {
				u64 old = m_data[i];
				u64 new_val = old | src.m_data[i];

				m_data[i] = new_val;
				changes |= (old ^ new_val);
			}

			return static_cast<bool>(changes);
		}

		void copy(const dense_set& src) {
			ASSERT(m_capacity >= src.m_capacity, "panic");
			std::ranges::copy(src.m_data.begin(), src.m_data.end(), m_data.begin());
		}

		void put(u64 index) {
			u64 slot = index / 64;
			u64 pos = index % 64;

			if (slot >= m_data.size()) {
				m_data.resize((index * 2 + 63) / 64, 0);
				m_capacity = index * 2;
			}

			m_data[slot] |= (1ull << pos);
		}

		void remove(u64 index) {
			u64 slot = index / 64;
			u64 pos = index % 64;

			if (slot < m_data.size()) {
				m_data[slot] &= ~(1ull << pos);
			}
		}

		bool get(u64 index) const {
			u64 slot = index / 64;
			u64 pos = index % 64;

			if (slot >= m_data.size()) {
				return false;
			}

			return m_data[slot] & (1ull << pos);
		}

		u64 data(u64 index) const {
			return m_data[index];
		}

		u64& data(u64 index) {
			return m_data[index];
		}

		u64 capacity() const {
			return m_capacity;
		}
	private:
		std::vector<u64> m_data;
		u64 m_capacity;
	};

	template <typename function>
	void foreach_set(const dense_set& ds, function func) {
		for (u64 i = 0; i < (ds.capacity() + 63) / 64; ++i) {
			u64 bits = ds.data(i);

			for (u64 it = i * 64; bits; bits >>= 1, ++it) {
				if (bits & 1) {
					func(it);
				}
			}
		}
	}
} // namespace utility
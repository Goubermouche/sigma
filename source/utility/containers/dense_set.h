#pragma once
#include "utility/containers/contiguous_container.h"

namespace utility {
	class dense_set : public contiguous_container<u64> {
	public:
		dense_set() = default;
		dense_set(u64 capacity);

		void clear();

		auto set_union(const dense_set& src) -> bool;
		void copy(dense_set& src);

		void put(u64 index);
		void remove(u64 index) const;
		auto get(u64 index) const -> bool;

		auto data(u64 index) const -> u64;
		auto data(u64 index) -> u64&;

		auto capacity() const ->u64;
	private:
		u64 m_used_capacity;
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
}

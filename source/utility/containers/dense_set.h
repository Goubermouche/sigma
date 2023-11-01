#pragma once
#include "utility/macros.h"

namespace utility {
	// TODO: add documentation
	// TODO: explore alternate datastructures which may result
	//       in better performance/cleaner code

	class dense_set {
	public:
		dense_set() = default;
		dense_set(u64 capacity);

		void clear();

		bool set_union(const dense_set& src);
		void copy(const dense_set& src);

		void put(u64 index);
		void remove(u64 index);
		bool get(u64 index) const;

		u64 data(u64 index) const;
		u64& data(u64 index);

		u64 capacity() const;
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
}

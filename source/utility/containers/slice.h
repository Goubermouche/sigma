#pragma once
#include "utility/types.h"

namespace utility {
	template<typename type>
	class slice {
	public:
		using iterator = type*;
		using const_iterator = const type*;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		slice() = default;
		slice(void* data, u64 size) :
			m_data(static_cast<type*>(data)), m_size(size) {}

		template<typename allocator>
		slice(allocator& alloc, u64 count) :
			m_data(static_cast<type*>(alloc.allocate(sizeof(type) * count))),
			m_size(count) {}

		type* get_data() const {
			return m_data;
		}

		u64 get_size() const {
			return m_size;
		}

		type& operator[](u64 index) {
			return m_data[index];
		}

		const type& operator[](u64 index) const {
			return m_data[index];
		}

		iterator begin() {
			return m_data;
		}

		const_iterator begin() const {
			return m_data;
		}

		reverse_iterator rbegin() {
			return reverse_iterator(end());
		}

		const_reverse_iterator rbegin() const {
			return const_reverse_iterator(end());
		}

		const_iterator end() const {
			return m_data + m_size;
		}

		iterator end() {
			return m_data + m_size;
		}

		reverse_iterator rend() {
			return reverse_iterator(begin());
		}

		const_reverse_iterator rend() const {
			return const_reverse_iterator(begin());
		}
	private:
		type* m_data;
		u64 m_size;
	};
}
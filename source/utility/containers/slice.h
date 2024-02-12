#pragma once
#include "utility/macros.h"

namespace utility {
	template<typename type, typename size_type = u64>
	class slice {
	public:
		using element_type = type;
		using iterator = type*;
		using const_iterator = const type*;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		slice() = default;
		slice(void* data, size_type size) :
			m_data(static_cast<type*>(data)), m_size(size) {}

		slice(type* data, size_type size) :
			m_data(data), m_size(size) {}

		template<typename allocator>
		slice(allocator& alloc, size_type count) :
			m_data(static_cast<type*>(alloc.allocate_zero(sizeof(type) * count))), m_size(count) {}

		[[nodiscard]] auto get_data() const -> type* {
			return m_data;
		}

		[[nodiscard]] auto get_size() const -> size_type {
			return m_size;
		}

		[[nodiscard]] auto is_empty() const -> bool {
			return m_size == 0;
		}

		[[nodiscard]] auto first() const -> type {
			return m_data[0];
		}

		[[nodiscard]] auto last() const -> type {
			return m_data[m_size - 1];
		}

		[[nodiscard]] auto operator[](u64 index) -> type& {
			return m_data[index];
		}

		[[nodiscard]] auto operator[](u64 index) const -> const type& {
			return m_data[index];
		}

		[[nodiscard]] auto begin() -> iterator {
			return m_data;
		}

		[[nodiscard]] auto begin() const -> const_iterator {
			return m_data;
		}

		[[nodiscard]] auto rbegin() -> reverse_iterator {
			return reverse_iterator(end());
		}

		[[nodiscard]] auto rbegin() const -> const_reverse_iterator {
			return const_reverse_iterator(end());
		}

		[[nodiscard]] auto end() const -> const_iterator {
			return m_data + m_size;
		}

		[[nodiscard]] auto end() -> iterator {
			return m_data + m_size;
		}

		[[nodiscard]] auto rend() -> reverse_iterator{
			return reverse_iterator(begin());
		}

		[[nodiscard]] auto rend() const -> const_reverse_iterator {
			return const_reverse_iterator(begin());
		}

		bool operator==(const slice& other) const {
			if(m_size != other.m_size) {
				return false;
			}

			for(size_type i = 0; i < m_size; ++i) {
				if(m_data[i] != other.m_data[i]) {
					return false;
				}
			}

			return true;
		}
	protected:
		type* m_data;
		size_type m_size;
	};

	template<typename type, typename size_type = u64>
	void copy(slice<type, size_type>& destination, const std::vector<type>& source) {
		ASSERT(destination.get_size() >= source.size(), "incompatible sizes");
		std::memcpy(destination.get_data(), source.data(), source.size() * sizeof(type));
	}

	template<typename type, typename size_type = u64>
	void copy(slice<type, size_type>& destination, u64 begin_offset, const std::vector<type>& source) {
		ASSERT(destination.get_size() + begin_offset >= source.size(), "incompatible sizes");
		std::memcpy(destination.get_data() + begin_offset, source.data(), source.size() * sizeof(type));
	}
} // namespace utility

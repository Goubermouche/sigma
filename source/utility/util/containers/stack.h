#pragma once
#include "contiguous_buffer.h"

namespace utility {
	template<typename type>
	class stack : public contiguous_buffer<type> {
	public:
		stack()
			: contiguous_buffer<type>() {}

		stack(const std::initializer_list<type>& initializer_list)
			: contiguous_buffer<type>(initializer_list) {}

		stack(const contiguous_buffer<type>& container)
			: contiguous_buffer<type>(container) {}

		[[nodiscard]] constexpr auto pop_back() -> type& {
			return this->m_data[--this->m_size];
		}

		constexpr void pop() {
			--this->m_size;
		}
	private:

	};
} // namespace utility

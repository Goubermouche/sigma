#pragma once
#include "utility/containers/contiguous_container.h"

namespace utility {
	template<typename type>
	class stack : public contiguous_container<type> {
	public:
		stack()
			: contiguous_container<type>() {}

		stack(const std::initializer_list<type>& initializer_list)
			: contiguous_container<type>(initializer_list) {}

		stack(const contiguous_container<type>& container)
			: contiguous_container<type>(container) {}

		[[nodiscard]] constexpr auto pop_back() -> type& {
			return this->m_data[--this->m_size];
		}

		constexpr void pop() {
			--this->m_size;
		}
	private:

	};
} // utility

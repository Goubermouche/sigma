#pragma once
#include "utility/containers/handle.h"

namespace utility {
	struct empty_property {};

	template<typename... properties>
	class property {
	public:
		template<typename type>
		auto get() -> type& {
			static_assert(
				std::disjunction_v<std::is_same<type, properties>...>,
				"type not contained within defined properties"
			);

			return *static_cast<type*>(m_property);
		}

		void set_property(void* value) {
			m_property = value;
		}
	protected:
		void* m_property = nullptr;
	};
}

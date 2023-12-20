#include "variable_registry.h"

namespace sigma::detail {
	variable_registry::variable_registry(ir::builder& builder) : m_builder(builder) {}

	auto variable_registry::register_variable(
		utility::string_table_key identifier_key, u16 size, u16 alignment
	) -> handle<ir::node> {
		return m_variables[identifier_key] = m_builder.create_local(size, alignment);
	}

	auto variable_registry::create_load(
		utility::string_table_key identifier_key, ir::data_type type, u16 alignment
	) -> handle<ir::node> {
		const auto it = m_variables.find(identifier_key);
		if(it != m_variables.end()) {
			return m_builder.create_load(it->second, type, alignment, false);
		}

		return nullptr;
	}

	void variable_registry::create_store(
		utility::string_table_key identifier_key, handle<ir::node> value, u16 alignment
	) {
		const auto it = m_variables.find(identifier_key);
		if (it != m_variables.end()) {
			m_builder.create_store(it->second, value, alignment, false);
			return;
		}

		ASSERT(false, "unknown variable referenced");
	}
} // namespace sigma::detail

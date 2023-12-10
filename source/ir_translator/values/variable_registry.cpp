#include "variable_registry.h"

namespace sigma::detail {
	variable_registry::variable_registry(sigma::ir::builder& builder) : m_builder(builder) {}

	auto variable_registry::register_variable(
		utility::symbol_table_key identifier_key, u16 size, u16 alignment
	) -> handle<sigma::ir::node> {
		return m_variables[identifier_key] = m_builder.create_local(size, alignment);
	}

	auto variable_registry::create_load(
		utility::symbol_table_key identifier_key, sigma::ir::data_type type, u16 alignment
	) -> handle<sigma::ir::node> {
		const auto it = m_variables.find(identifier_key);
		if(it != m_variables.end()) {
			return m_builder.create_load(it->second, type, alignment, false);
		}

		return nullptr;
	}

	void variable_registry::create_store(
		utility::symbol_table_key identifier_key, handle<sigma::ir::node> value, u16 alignment
	) {
		const auto it = m_variables.find(identifier_key);
		if (it != m_variables.end()) {
			m_builder.create_store(it->second, value, alignment, false);
		}

		ASSERT(false, "unknown variable referenced");
	}
} // namespace sigma::detail


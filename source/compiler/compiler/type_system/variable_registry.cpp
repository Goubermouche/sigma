#include "variable_registry.h"
#include "compiler/compiler/compilation_context.h"

namespace sigma {
  variable_registry::variable_registry(backend_context& context) : m_context(context) {}

	void variable_registry::pre_declare_variable(utility::string_table_key identifier, data_type type) {
		m_variables[identifier].type = type;
	}

	auto variable_registry::declare_variable(utility::string_table_key identifier, u16 size, u16 alignment) -> handle<ir::node> {
		return m_variables[identifier].value = m_context.builder.create_local(size, alignment);
	}

	auto variable_registry::get_variable(utility::string_table_key identifier) -> handle<variable> {
		const auto it = m_variables.find(identifier);
		if(it != m_variables.end()) {
			return &it->second;
		}

		return nullptr;
	}

	auto variable_registry::create_load(utility::string_table_key identifier, ir::data_type type, u16 alignment) -> handle<ir::node> {
		const auto it = m_variables.find(identifier);
		if (it != m_variables.end()) {
			return m_context.builder.create_load(it->second.value, type, alignment, false);
		}

		return nullptr;
  }

	void variable_registry::create_store(utility::string_table_key identifier, handle<ir::node> value, u16 alignment) {
		const auto it = m_variables.find(identifier);
		if (it != m_variables.end()) {
			m_context.builder.create_store(it->second.value, value, alignment, false);
			return;
		}

		ASSERT(false, "unknown variable referenced");
  }
} // namespace sigma

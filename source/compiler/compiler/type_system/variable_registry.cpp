#include "variable_registry.h"
#include "compiler/compiler/compilation_context.h"

namespace sigma {
  variable_registry::variable_registry(backend_context& context) : m_context(context) {
		reset_active_scope();
  }

  void variable_registry::push_scope() {
		const handle<scope> old = m_active_scope;
		m_active_scope->children.push_back({});
		m_active_scope = &m_active_scope->children.back();
		m_active_scope->parent = old;
  }

	void variable_registry::pop_scope() {
		ASSERT(m_active_scope->parent, "invalid pop on global scope");
		m_active_scope = m_active_scope->parent;
	}

	void variable_registry::trace_push_scope() {
		m_active_scope = &m_active_scope->children[m_active_scope->trace_index++];
	}

	void variable_registry::trace_pop_scope() {
		m_active_scope = m_active_scope->parent;
	}

	void variable_registry::reset_active_scope() {
		m_active_scope = &m_global_scope;
	}

	void variable_registry::pre_declare_variable(utility::string_table_key identifier, data_type type) const {
		m_active_scope->variables[identifier].type = type;
	}

	auto variable_registry::declare_variable(utility::string_table_key identifier, u16 size, u16 alignment) const -> handle<ir::node> {
		return m_active_scope->variables.at(identifier).value = m_context.builder.create_local(size, alignment);
	}

	auto variable_registry::get_variable(utility::string_table_key identifier) const -> handle<variable> {
		return m_active_scope->get_variable(identifier);
	}

	bool variable_registry::contains(utility::string_table_key identifier) const {
		return m_active_scope->contains(identifier);
	}

	auto variable_registry::create_load(utility::string_table_key identifier, ir::data_type type, u16 alignment) const -> handle<ir::node> {
		const handle<variable> variable = m_active_scope->get_variable(identifier);
		if(variable != nullptr) {
			return m_context.builder.create_load(variable->value, type, alignment, false);
		}

		return nullptr;
  }

	void variable_registry::create_store(utility::string_table_key identifier, handle<ir::node> value, u16 alignment) const {
		const handle<variable> variable = m_active_scope->get_variable(identifier);
		if (variable != nullptr) {
			m_context.builder.create_store(variable->value, value, alignment, false);
			return;
		}

		ASSERT(false, "unknown variable referenced");
  }

	auto variable_registry::scope::get_variable(utility::string_table_key identifier) -> handle<variable> {
		const auto it = variables.find(identifier);
		if (it != variables.end()) {
			return &it->second;
		}

		// search the parent scope
		if (parent) {
			return parent->get_variable(identifier);
		}

		return nullptr;
	}

	bool variable_registry::scope::contains(utility::string_table_key identifier) const {
		if(variables.contains(identifier)) {
			return true;
		}

		// search the parent scope
		if(parent) {
			return parent->contains(identifier);
		}

		// no parent, the variable doesn't exist
		return false;
	}
} // namespace sigma

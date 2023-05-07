#include "function_registry.h"

namespace channel {
	function_ptr function_registry::get_function(const std::string& identifier) const {
		const auto it = m_functions.find(identifier);
		if(it != m_functions.end()) {
			return it->second;
		}

		return nullptr;
	}

	function_declaration_ptr function_registry::get_function_declaration(const std::string& identifier) const {
		// try and find an external function declaration
		auto it = m_external_function_declarations.find(identifier);
		if (it != m_external_function_declarations.end()) {
			return it->second;
		}

		// try and find a regular declaration
		it = m_function_declarations.find(identifier);
		if (it != m_function_declarations.end()) {
			return it->second;
		}

		return nullptr;
	}

	const std::unordered_map<std::string, function_declaration_ptr>& function_registry::get_external_function_declarations() const {
		return m_external_function_declarations;
	}

	void function_registry::insert_function(const std::string& identifier, function_ptr function) {
		m_functions[identifier] = function;
	}

	void function_registry::insert_function_declaration(const std::string& identifier, function_declaration_ptr function) {
		m_function_declarations[identifier] = function;
	}

	bool function_registry::contains_function(const std::string& identifier) const {
		return m_functions.contains(identifier) || m_external_function_declarations.contains(identifier);
	}

	bool function_registry::contains_function_declaration(const std::string& identifier) const {
		return m_function_declarations.contains(identifier) || m_external_function_declarations.contains(identifier);
	}
}
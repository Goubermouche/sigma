#include "variable_type_scope.h"

namespace channel {
	variable_type_scope::variable_type_scope(variable_type_scope_ptr parent)
		: m_parent(parent) {}

	void variable_type_scope::insert_variable_type(const std::string& identifier, type type) {
		m_variable_types[identifier] = type;
	}

	bool variable_type_scope::contains_variable_type(const std::string& identifier) const {
		// check in the current scopes
		if(m_variable_types.contains(identifier)) {
			return true;
		}

		// if we have a parent scope check in there as well
		if (m_parent != nullptr) {
			return m_parent->contains_variable_type(identifier);
		}

		// variable does not exist in the current scope
		return false;
	}

	type variable_type_scope::get_variable_type(const std::string& identifier) const {
		const auto it = m_variable_types.find(identifier); // try to find an llvm::Value in this scope

		// if we've found a value in this scope, return it
		if (it != m_variable_types.end()) {
			return it->second;
		}

		// if we haven't found a value, but we have a parent scope we need to search it as well 
		if (m_parent != nullptr) {
			return m_parent->get_variable_type(identifier);
		}

		// variable does not exist in current scope
		return type::unknown();
	}
}
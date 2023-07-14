#include "variable_registry.h"

namespace sigma {
	bool variable_registry::insert(
		const std::string& variable_name, 
		value_ptr value
	) {
		return m_variables.insert({ variable_name, value }).second;
	}

	bool variable_registry::contains(
		const std::string& variable_name
	) const {
		return m_variables.contains(variable_name);
	}

	value_ptr variable_registry::get(
		const std::string& variable_name
	) {
		return m_variables[variable_name];
	}
}
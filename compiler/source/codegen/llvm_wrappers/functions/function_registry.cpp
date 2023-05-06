#include "function_registry.h"

namespace channel {
	function_ptr function_registry::get_function(const std::string& identifier) const {
		const auto it = m_functions.find(identifier); 

		if(it != m_functions.end()) {
			return it->second;
		}

		return nullptr;
	}

	void function_registry::insert_function(const std::string& identifier, function_ptr function) {
		m_functions[identifier] = function;
	}

	bool function_registry::contains_function(const std::string& identifier) const {
		return m_functions.contains(identifier);
	}
}
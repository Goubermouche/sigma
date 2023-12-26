#include "function_registry.h"

namespace sigma::detail {
	function_registry::function_registry(ir::builder& builder) : m_builder(builder) {}

	auto function_registry::create_call(
		utility::string_table_key identifier_key, const std::vector<handle<ir::node>>& parameters
	) -> handle<ir::node> {
		// attempt to call a local function
		const auto func_it = m_functions.find(identifier_key);
		if (func_it != m_functions.end()) {
			return m_builder.create_call(func_it->second, parameters);
		}

		// attempt to call an external function
		const auto external_func_it = m_external_functions.find(identifier_key);
		if (external_func_it != m_external_functions.end()) {
			return m_builder.create_call(
				external_func_it->second.external, external_func_it->second.signature, parameters
			);
		}

		return nullptr;
	}
} // sigma::detail

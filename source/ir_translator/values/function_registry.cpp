#include "function_registry.h"

namespace sigma::detail {
	function_registry::function_registry(sigma::ir::builder& builder) : m_builder(builder) {}

	void function_registry::register_function(
		utility::symbol_table_key identifier_key, const sigma::ir::function_signature& function_sig
	) {
		m_functions[identifier_key] = m_builder.create_function(function_sig, sigma::ir::linkage::PUBLIC);
	}

	void function_registry::register_external_function(
		utility::symbol_table_key identifier_key, const sigma::ir::function_signature& function_sig
	) {
		m_external_functions[identifier_key] = external_function{
			.external = m_builder.create_external(function_sig, sigma::ir::linkage::SO_LOCAL),
			.signature = function_sig
		};
	}

	auto function_registry::create_call(
		utility::symbol_table_key identifier_key, const std::vector<handle<sigma::ir::node>>& parameters
	) -> handle<sigma::ir::node> {
		// attempt to call a local function
		const auto func_it = m_functions.find(identifier_key);
		if (func_it != m_functions.end()) {
			return m_builder.create_call(func_it->second, parameters).front();
		}

		// attempt to call an external function
		const auto external_func_it = m_external_functions.find(identifier_key);
		if (external_func_it != m_external_functions.end()) {
			return m_builder.create_call(external_func_it->second.external, external_func_it->second.signature, parameters).front();
		}

		return nullptr;
	}
} // sigma::detail
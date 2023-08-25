#include "function.h"

namespace sigma {
	function_declaration::function_declaration(
		type return_type,
		const std::vector<std::pair<std::string, type>>& arguments,
		bool is_variadic,
		const std::string& external_function_name,
		const utility::file_range& range
	) : m_return_type(return_type),
	m_arguments(arguments),
	m_is_variadic(is_variadic),
	m_external_function_name(external_function_name),
	m_range(range) {}

	const std::vector<std::pair<std::string, type>>& function_declaration::get_arguments() const {
		return m_arguments;
	}

	bool function_declaration::is_variadic() const {
		return m_is_variadic;
	}

	const std::string& function_declaration::get_external_function_name() const {
		return m_external_function_name;
	}

	const utility::file_range& function_declaration::get_range() const {
		return m_range;
	}

	type function_declaration::get_return_type() const {
		return m_return_type;
	}

	function::function(
		type return_type,
		llvm::Function* function,
		const std::vector<std::pair<std::string, type>>& arguments,
		bool is_variadic,
		const utility::file_range& range
	) : function_declaration(
		return_type, 
		arguments, 
		is_variadic,
		"", 
		range
	), m_value(function) {}

	llvm::Function* function::get_function() const {
		return m_value;
	}
}

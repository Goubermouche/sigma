#include "function_call_node.h"

namespace channel {
	function_call_node::function_call_node(
		const token_position& position,
		const std::string& function_identifier,
		const std::vector<node_ptr>& function_arguments
	) : node(position),
	m_function_name(function_identifier),
	m_function_arguments(function_arguments) {}

	acceptation_result function_call_node::accept(visitor& visitor, const codegen_context& context) {
		return visitor.visit_function_call_node(*this, context);
	}

	void function_call_node::print(u64 depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, "function call",	is_last);
		console::out
			<< "'"
			<< AST_NODE_VARIABLE_COLOR
			<< m_function_name
			<< color::white
			<< "'\n";

		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print function argument statements 
		for (u64 i = 0; i < m_function_arguments.size(); ++i) {
			m_function_arguments[i]->print(depth + 1, new_prefix, i == m_function_arguments.size() - 1);
		}
	}

	const std::string& function_call_node::get_function_identifier() const	{
		return m_function_name;
	}

	const std::vector<node_ptr>& function_call_node::get_function_arguments() const	{
		return m_function_arguments;
	}
}

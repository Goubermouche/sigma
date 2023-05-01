#include "function_call_node.h"

namespace channel {
	function_call_node::function_call_node(
		u64 line_number, 
		const std::string& function_identifier,
		const std::vector<node_ptr>& function_arguments
	) : node(line_number),
	m_function_name(function_identifier),
	m_function_arguments(function_arguments) {}

	bool function_call_node::accept(visitor& visitor, value_ptr& out_value) {
		LOG_NODE_NAME(function_call_node);
		return visitor.visit_function_call_node(*this, out_value);
	}

	void function_call_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, "function call", "'" + m_function_name + "'\n",	is_last);
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

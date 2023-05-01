#include "function_node.h"

namespace channel {
	function_node::function_node(
		u64 line_number, 
		const type& function_return_type, 
		const std::string& function_identifier,
		const std::vector<std::pair<std::string, type>>& function_arguments,
		const std::vector<node_ptr>& function_statements
	) : node(line_number),
	m_function_return_type(function_return_type),
	m_function_identifier(function_identifier),
	m_function_arguments(function_arguments),
	m_function_statements(function_statements) {}

	bool function_node::accept(visitor& visitor, value_ptr& out_value) {
		LOG_NODE_NAME(function_node);
		return visitor.visit_function_node(*this, out_value);
	}

	void function_node::print(int depth, const std::wstring& prefix, bool is_last) {
		// print the function name and return type
		print_value(
			depth,
			prefix,
			"function declaration",
			"'" + m_function_return_type.to_string() + "' '" + m_function_identifier + "' (", is_last
		);

		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print function arguments 
		for (u64 i = 0; i < m_function_arguments.size(); ++i) {
			console::out << s_ws_to_s_converter.from_bytes(m_function_arguments[i].second.to_string()) << (i == m_function_arguments.size() - 1 ? "" : ", ");
		}

		console::out  << L")\n";

		// print inner statements 
		for (u64 i = 0; i < m_function_statements.size(); ++i) {
			m_function_statements[i]->print(depth + 1, new_prefix, i == m_function_statements.size() - 1);
		}
	}

	const type& function_node::get_function_return_type() const {
		return m_function_return_type;
	}

	const std::string& function_node::get_function_identifier() const {
		return m_function_identifier;
	}

	const std::vector<node_ptr>& function_node::get_function_statements() const	{
		return m_function_statements;
	}

	const std::vector<std::pair<std::string, type>>& function_node::get_function_arguments() const {
		return m_function_arguments;
	}
}

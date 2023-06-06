#include "function_node.h"

namespace channel {
	function_node::function_node(
		const token_location& location,
		const type& function_return_type,
		bool is_var_arg,
		const std::string& function_identifier,
		const std::vector<std::pair<std::string, type>>& function_arguments,
		const std::vector<node_ptr>& function_statements
	) : node(location),
	m_function_return_type(function_return_type),
	m_function_identifier(function_identifier),
	m_is_var_arg(is_var_arg),
	m_function_arguments(function_arguments),
	m_function_statements(function_statements) {}

	expected_value function_node::accept(code_generator& visitor, const code_generation_context& context) {
		return visitor.visit_function_node(*this, context);
	}

	void function_node::print(u64 depth, const std::wstring& prefix, bool is_last) {
		// print the function name and return type
		print_node_name(
			depth,
			prefix,
			"function declaration",
			is_last
		);

		console::out
			<< "'"
			<< AST_NODE_TYPE_COLOR
			<< m_function_return_type.to_string()
			<< color::white
			<< "' '"
			<< AST_NODE_VARIABLE_COLOR
			<< m_function_identifier
			<< color::white
			<< "' (";

		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print function arguments 
		for (u64 i = 0; i < m_function_arguments.size(); ++i) {
			console::out
				<< AST_NODE_TYPE_COLOR
				<< m_function_arguments[i].second.to_string()
				<< color::white
				<< (i == m_function_arguments.size() - 1 ? "" : ", ");
		}

		console::out  << ")\n";

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

	bool function_node::is_var_arg() const {
		return m_is_var_arg;
	}

	const std::vector<node_ptr>& function_node::get_function_statements() const	{
		return m_function_statements;
	}

	const std::vector<std::pair<std::string, type>>& function_node::get_function_arguments() const {
		return m_function_arguments;
	}
}

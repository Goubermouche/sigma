#include "local_declaration_node.h"

namespace channel {
	local_declaration_node::local_declaration_node(
		u64 line_number,
		const type& declaration_type,
		const std::string& declaration_identifier, 
		const node_ptr& expression_node
	) : declaration_node(
		line_number, 
		declaration_type,
		declaration_identifier,
		expression_node
	) {}

	bool local_declaration_node::accept(visitor& visitor, value_ptr& out_value) {
		LOG_NODE_NAME(local_declaration_node);
		return visitor.visit_local_declaration_node(*this, out_value);
	}

	void local_declaration_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(
			depth, 
			prefix,
			"local declaration", 
			is_last
		);

		console::out
			<< "'"
			<< AST_NODE_VARIABLE_COLOR 
			<< get_declaration_identifier()
			<< color::white
			<< "' '"
			<< AST_NODE_TYPE_COLOR
			<< get_declaration_type().to_string()
			<< color::white
			<< "'\n";

		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statement
		const node_ptr expression = get_expression_node();

		if (expression != nullptr) {
			expression->print(depth + 1, new_prefix, true);
		}
	}
}

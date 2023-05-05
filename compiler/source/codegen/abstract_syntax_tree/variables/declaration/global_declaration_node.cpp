#include "global_declaration_node.h"

namespace channel {
	global_declaration_node::global_declaration_node(
		const token_position& position,
		const type& declaration_type,
		const std::string& declaration_identifier,
		const node_ptr& expression_node
	) : declaration_node(
		position,
		declaration_type, 
		declaration_identifier, 
		expression_node
	) {}

	bool global_declaration_node::accept(visitor& visitor, value_ptr& out_value)	{
		LOG_NODE_NAME(global_declaration_node);
		return visitor.visit_global_declaration_node(*this, out_value);
	}

	void global_declaration_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(
			depth, 
			prefix, 
			"global declaration", 
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

		if(expression != nullptr) {
			expression->print(depth + 1, new_prefix, true);
		}
	}
}
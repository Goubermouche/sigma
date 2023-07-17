#include "local_declaration_node.h"

namespace sigma {
	local_declaration_node::local_declaration_node(
		const file_position& position,
		const type& declaration_type,
		const std::string& declaration_identifier, 
		const node_ptr& expression_node
	) : declaration_node(
		position, 
		declaration_type,
		declaration_identifier,
		expression_node
	) {}

	outcome::result<value_ptr> local_declaration_node::accept(
		code_generator_template& visitor, 
		const code_generation_context& context
	) {
		return visitor.visit_local_declaration_node(*this, context);
	}

	void local_declaration_node::print(
		u64 depth,
		const std::wstring& prefix,
		bool is_last
	) {
		print_node_name(
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

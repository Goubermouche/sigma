#include "global_declaration_node.h"

namespace channel {
	global_declaration_node::global_declaration_node(u64 line_number, const type& declaration_type, const std::string& declaration_identifier, node* expression_node)
		: declaration_node(line_number, declaration_type, declaration_identifier, expression_node) {}

	bool global_declaration_node::accept(visitor& visitor, value*& out_value)	{
		LOG_NODE_NAME(global_declaration_node);
		return visitor.visit_global_declaration_node(*this, out_value);
	}

	void global_declaration_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, L"GlobalDecl", "'" + get_declaration_identifier() + "' '" + get_declaration_type().to_string() + "'\n", is_last);
		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statement
		node* expression = get_expression_node();

		if(expression != nullptr) {
			expression->print(depth + 1, new_prefix, true);
		}
	}
}
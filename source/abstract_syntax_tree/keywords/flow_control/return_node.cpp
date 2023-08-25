#include "return_node.h"

namespace sigma {
	return_node::return_node(
		const utility::file_range& range,
		const node_ptr& return_expression_node
	) : node(range),
	m_return_expression_node(return_expression_node) {}

	utility::outcome::result<value_ptr> return_node::accept(
		abstract_syntax_tree_visitor_template& visitor, 
		const code_generation_context& context
	) {
		return visitor.visit_return_node(*this, context);
	}

	void return_node::print(
		u64 depth,
		const std::wstring& prefix, 
		bool is_last
	) {
		print_node_name(depth, prefix, "return statement", is_last);
		utility::console::out << "\n";

		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print the inner statement
		if(m_return_expression_node) {
			m_return_expression_node->print(depth + 1, new_prefix, true);
		}
	}

	const node_ptr& return_node::get_return_expression_node() const {
		return m_return_expression_node;
	}
}
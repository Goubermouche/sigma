#include "translation_unit_node.h"

namespace channel {
	translation_unit_node::translation_unit_node(
		const std::vector<node_ptr>& nodes
	) : node({ "x", 0, 0}),
	m_nodes(nodes) {}

	acceptation_result translation_unit_node::accept(visitor& visitor, const codegen_context& context) {
		return visitor.visit_translation_unit_node(*this, context);
	}

	void translation_unit_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, "translation unit", is_last);
		console::out << "\n";

		const std::wstring new_prefix = get_new_prefix(depth, prefix, is_last);

		// print inner statements
		for (u64 i = 0; i < m_nodes.size(); ++i) {
			m_nodes[i]->print(depth + 1, new_prefix, i == m_nodes.size() - 1);
		}
	}

	const std::vector<node_ptr>& translation_unit_node::get_nodes() const {
		return m_nodes;
	}
}
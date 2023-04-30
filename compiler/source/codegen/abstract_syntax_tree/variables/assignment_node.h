#pragma once
#include "../node.h"

namespace channel {
	/**
	 * \brief AST node, assignment of a value.
	 */
	class assignment_node : public node {
	public:
		assignment_node(u64 line_number, node_ptr variable, node_ptr expression_node);
		bool accept(visitor& visitor, value_ptr& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		node_ptr get_variable_node() const;
		node_ptr get_expression_node() const;
	private:
		node_ptr m_variable_node;
		node_ptr m_expression_node;
	};
}
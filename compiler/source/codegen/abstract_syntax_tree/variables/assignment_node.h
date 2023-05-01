#pragma once
#include "../node.h"

namespace channel {
	/**
	 * \brief AST node, represents an assignment to a non-array variable.
	 */
	class assignment_node : public node {
	public:
		assignment_node(
			u64 line_number,
			const node_ptr& variable,
			const node_ptr& expression_node
		);

		bool accept(visitor& visitor, value_ptr& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		const node_ptr& get_variable_node() const;
		const node_ptr& get_expression_node() const;
	private:
		node_ptr m_variable_node;
		node_ptr m_expression_node;
	};
}
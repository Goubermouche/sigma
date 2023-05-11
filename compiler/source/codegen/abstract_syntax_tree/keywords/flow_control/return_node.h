#pragma once
#include "codegen/abstract_syntax_tree/node.h"

namespace channel {
	/**
	 * \brief AST node, represents a return statement.
	 */
	class return_node : public node {
	public:
		return_node(
			const token_position& position,
			const node_ptr& return_expression_node
		);

		acceptation_result accept(
			visitor& visitor,
			const codegen_context& context
		) override;

		void print(u64 depth, const std::wstring& prefix, bool is_last) override;

		const node_ptr& get_return_expression_node() const;
	private:
		node_ptr m_return_expression_node;
	};
}
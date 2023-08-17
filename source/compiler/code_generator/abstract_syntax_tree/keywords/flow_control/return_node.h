#pragma once
#include "code_generator/abstract_syntax_tree/node.h"

namespace sigma {
	/**
	 * \brief AST node, represents a return statement.
	 */
	class return_node : public node {
	public:
		return_node(
			const file_range& range,
			const node_ptr& return_expression_node
		);

		outcome::result<value_ptr> accept(
			code_generator_template& visitor,
			const code_generation_context& context
		) override;

		void print(
			u64 depth, 
			const std::wstring& prefix,
			bool is_last
		) override;

		const node_ptr& get_return_expression_node() const;
	private:
		node_ptr m_return_expression_node;
	};
}
#pragma once
#include "code_generator/abstract_syntax_tree/node.h"

namespace channel {
	/**
	 * \brief AST node, represents a break statement.
	 */
	class break_node : public node {
	public:
		break_node(const token_position& position);

		acceptation_result accept(
			code_generator& visitor,
			const codegen_context& context
		) override;

		void print(
			u64 depth,
			const std::wstring& prefix, 
			bool is_last
		) override;
	};
}
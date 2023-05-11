#pragma once
#include "codegen/abstract_syntax_tree/variables/declaration/declaration_node.h"

namespace channel {
	/**
	 * \brief AST node, represents the local declaration of a variable.
	 */
	class local_declaration_node : public declaration_node {
	public:
		local_declaration_node(
			const token_position& position,
			const type& declaration_type,
			const std::string& declaration_identifier,
			const node_ptr& expression_node = nullptr
		);

		bool accept(visitor& visitor, value_ptr& out_value, codegen_context context) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;
	};
}
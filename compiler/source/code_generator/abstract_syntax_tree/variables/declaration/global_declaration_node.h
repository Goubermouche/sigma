#pragma once
#include "code_generator/abstract_syntax_tree/variables/declaration/declaration_node.h"

namespace channel {
	/**
	 * \brief AST node, represents the global declaration of a variable.
	 */
	class global_declaration_node : public declaration_node {
	public:
		global_declaration_node(
			const token_position& position,
			const type& declaration_type,
			const std::string& declaration_identifier, 
			const node_ptr& expression_node = nullptr
		);

		acceptation_result accept(
			code_generator& visitor,
			const codegen_context& context
		) override;

		void print(u64 depth, const std::wstring& prefix, bool is_last) override;
	};
}
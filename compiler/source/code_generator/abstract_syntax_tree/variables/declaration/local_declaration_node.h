#pragma once
#include "code_generator/abstract_syntax_tree/variables/declaration/declaration_node.h"

namespace sigma {
	/**
	 * \brief AST node, represents the local declaration of a variable.
	 */
	class local_declaration_node : public declaration_node {
	public:
		local_declaration_node(
			const token_location& location,
			const type& declaration_type,
			const std::string& declaration_identifier,
			const node_ptr& expression_node = nullptr
		);

		expected_value accept(
			code_generator& visitor,
			const code_generation_context& context
		) override;

		void print(u64 depth, const std::wstring& prefix, bool is_last) override;
	};
}
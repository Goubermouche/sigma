#pragma once
#include "code_generator/abstract_syntax_tree/node.h"

namespace channel {
	/**
	 * \brief AST node, represents a top level translation unit node.
	 */
	class translation_unit_node : public node {
	public:
		translation_unit_node(
			const std::vector<node_ptr>& nodes
		);

		acceptation_result accept(
			code_generator& visitor, 
			const code_generation_context& context
		) override;

		void print(
			u64 depth,
			const std::wstring& prefix,
			bool is_last
		) override;

		const std::vector<node_ptr>& get_nodes() const;
	private:
		std::vector<node_ptr> m_nodes;
	};
}
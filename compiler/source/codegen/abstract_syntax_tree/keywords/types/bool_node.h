#pragma once
#include "codegen/abstract_syntax_tree/node.h"

namespace channel {
	/**
	 * \brief AST node, represents a boolean literal.
	 */
	class bool_node : public node {
	public:
		bool_node(
			const token_position& position,
			bool value
		);

		acceptation_result accept(
			visitor& visitor,
			const codegen_context& context
		) override;

		void print(u64 depth, const std::wstring& prefix, bool is_last) override;

		bool get_value() const;
	private:
		bool m_value;
	};
}
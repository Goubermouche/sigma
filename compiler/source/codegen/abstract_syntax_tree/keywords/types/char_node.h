#pragma once
#include "codegen/abstract_syntax_tree/node.h"

namespace channel {
	/**
	 * \brief AST node, represents a char literal.
	 */
	class char_node : public node {
	public:
		char_node(
			const token_position& position, 
			char value
		);

		acceptation_result accept(
			visitor& visitor,
			const codegen_context& context
		) override;

		void print(int depth, const std::wstring& prefix, bool is_last) override;

		char get_value() const;
	private:
		char m_value;
	};
}

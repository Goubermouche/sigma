#pragma once
#include "code_generator/abstract_syntax_tree/node.h"

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
			code_generator& visitor,
			const code_generation_context& context
		) override;

		void print(
			u64 depth, 
			const std::wstring& prefix, 
			bool is_last
		) override;

		char get_value() const;
	private:
		char m_value;
	};
}

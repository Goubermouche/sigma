#pragma once
#include "code_generator/abstract_syntax_tree/node.h"

namespace sigma {
	/**
	 * \brief AST node, represents a break statement.
	 */
	class break_node : public node {
	public:
		break_node(const file_range& range);

		outcome::result<value_ptr> accept(
			code_generator_template& visitor,
			const code_generation_context& context
		) override;

		void print(
			u64 depth,
			const std::wstring& prefix, 
			bool is_last
		) override;
	};
}
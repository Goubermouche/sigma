#pragma once
#include "code_generator/abstract_syntax_tree/node.h"

namespace sigma {
	/**
	 * \brief AST node, represents a char literal.
	 */
	class char_node : public node {
	public:
		char_node(
			const file_position& location, 
			char value
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

		char get_value() const;
	private:
		char m_value;
	};
}

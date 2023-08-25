#pragma once
#include "abstract_syntax_tree/node.h"

namespace sigma {
	/**
	 * \brief AST node, represents a char literal.
	 */
	class char_node : public node {
	public:
		char_node(
			const utility::file_range& range,
			char value
		);

		utility::outcome::result<value_ptr> accept(
			abstract_syntax_tree_visitor_template& visitor,
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

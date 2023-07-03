#pragma once
#include "code_generator/abstract_syntax_tree/node.h"

namespace sigma {
	/**
	 * \brief AST node, represents a boolean literal.
	 */
	class bool_node : public node {
	public:
		bool_node(
			const token_location& location,
			bool value
		);

		expected_value accept(
			code_generator& visitor,
			const code_generation_context& context
		) override;

		void print(
			u64 depth,
			const std::wstring& prefix, 
			bool is_last
		) override;

		bool get_value() const;
	private:
		bool m_value;
	};
}
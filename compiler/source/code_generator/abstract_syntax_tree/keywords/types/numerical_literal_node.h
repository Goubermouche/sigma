#pragma once
#include "code_generator/abstract_syntax_tree/node.h"

namespace sigma {
	/**
	 * \brief AST node, represents a numerical literal.
	 */
	class numerical_literal_node : public node {
	public:
		numerical_literal_node(
			const token_location& location,
			const std::string& value,
			type preferred_type
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

		const std::string& get_value() const;
		const type& get_preferred_type() const;
	private:
		std::string m_value;
		type m_preferred_type;
	};
}
#pragma once
#include "codegen/abstract_syntax_tree/node.h"

namespace channel {
	/**
	 * \brief AST node, represents a string literal.
	 */
	class string_node : public node {
	public:
		string_node(
			const token_position& position,
			const std::string& value
		);

		acceptation_result accept(
			visitor& visitor,
			const codegen_context& context
		) override;

		void print(int depth, const std::wstring& prefix, bool is_last) override;

		const std::string& get_value() const;
	private:
		std::string m_value;
	};
}

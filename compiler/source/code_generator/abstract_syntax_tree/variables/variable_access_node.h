#pragma once
#include "code_generator/abstract_syntax_tree/node.h"

namespace channel {
	/**
	 * \brief AST node, represents a load of a variable
	 */
	class variable_access_node : public node {
	public:
		variable_access_node(
			const token_position& position,
			const std::string& variable_identifier
		);

		acceptation_result accept(
			code_generator& visitor,
			const code_generation_context& context
		) override;

		void print(u64 depth, const std::wstring& prefix, bool is_last) override;

		const std::string& get_variable_identifier() const;
	private:
		std::string m_variable_identifier;
	};
}
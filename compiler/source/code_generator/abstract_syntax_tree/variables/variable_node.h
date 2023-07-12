#pragma once
#include "code_generator/abstract_syntax_tree/node.h"

namespace sigma {
	/**
	 * \brief AST node, represents an access to an existing named value.
	 */
	class variable_node : public node {
	public:
		variable_node(
			const file_position& location,
			const std::string& variable_identifier
		);

		outcome::result<value_ptr> accept(
			code_generator& visitor,
			const code_generation_context& context
		) override;

		void print(
			u64 depth,
			const std::wstring& prefix, 
			bool is_last
		) override;

		const std::string& get_variable_identifier() const;
	private:
		std::string m_variable_identifier;
	};
}
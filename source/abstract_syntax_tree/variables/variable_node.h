#pragma once
#include "abstract_syntax_tree/node.h"

namespace sigma {
	/**
	 * \brief AST node, represents an access to an existing named value.
	 */
	class variable_node : public node {
	public:
		variable_node(
			const file_range& range,
			const std::string& identifier
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

		const std::string& get_identifier() const;
	private:
		std::string m_identifier;
	};
}
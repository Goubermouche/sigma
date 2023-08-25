#pragma once
#include "abstract_syntax_tree/node.h"

namespace sigma {
	/**
	 * \brief AST node, represents a load of a variable
	 */
	class variable_access_node : public node {
	public:
		variable_access_node(
			const utility::file_range& range,
			const std::string& identifier
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

		const std::string& get_identifier() const;
	private:
		std::string m_identifier;
	};
}

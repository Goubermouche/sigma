#pragma once
#include "abstract_syntax_tree/node.h"

namespace sigma {
	/**
	 * \brief AST node, represents a call to a given function. 
	 */
	class function_call_node : public node {
	public:
		function_call_node(
			const utility::file_range& range,
			const std::string& function_identifier,
			const std::vector<node_ptr>& function_arguments
		);

		utility::outcome::result<value_ptr> accept(
			abstract_syntax_tree_visitor_template& visitor,
			const code_generation_context& context
		) override;

		void print(u64 depth, const std::wstring& prefix, bool is_last) override;

		const std::string& get_function_identifier() const;
		const std::vector<node_ptr>& get_function_arguments() const;
	private:
		std::string m_function_name;
		std::vector<node_ptr> m_function_arguments;
	};
}

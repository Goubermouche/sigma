#pragma once
#include "abstract_syntax_tree/node.h"

namespace sigma {
	/**
	 * \brief AST node, represents a string literal.
	 */
	class string_node : public node {
	public:
		string_node(
			const file_range& range,
			const std::string& value
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

		const std::string& get_value() const;
	private:
		std::string m_value;
	};
}

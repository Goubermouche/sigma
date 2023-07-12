#pragma once
#include "code_generator/abstract_syntax_tree/node.h"

namespace sigma {
	/**
	 * \brief AST node, represents a top level file include node.
	 */
	class file_include_node : public node {
	public:
		file_include_node(
			const file_position& location,
			const filepath& path
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

		const filepath& get_path() const;
	private:
		filepath m_path;
	};
}

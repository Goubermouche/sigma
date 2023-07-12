#include "file_include_node.h"

namespace sigma {
	file_include_node::file_include_node(
		const file_position& location,
		const filepath& path
	) : node(location),
	m_path(path) {}

	outcome::result<value_ptr> file_include_node::accept(
		code_generator& visitor,
		const code_generation_context& context
	) {
		return visitor.visit_file_include_node(*this, context);
	}

	void file_include_node::print(
		u64 depth, 
		const std::wstring& prefix,
		bool is_last
	) {
		print_node_name(
			depth,
			prefix,
			"file include",
			is_last
		);

		console::out
			<< color::white
			<< '\''
			<< AST_NODE_TEXT_LITERAL_COLOR
			<< escape_string(m_path.string())
			<< color::white
			<< "'\n";
	}

	const filepath& file_include_node::get_path() const {
		return m_path;
	}
}
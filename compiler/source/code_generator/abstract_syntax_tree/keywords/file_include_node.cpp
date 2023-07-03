#include "file_include_node.h"

namespace sigma {
	file_include_node::file_include_node(
		const token_location& location,
		const std::string& filepath
	) : node(location),
	m_filepath(filepath) {}

	expected_value file_include_node::accept(code_generator& visitor, const code_generation_context& context) {
		return visitor.visit_file_include_node(*this, context);
	}

	void file_include_node::print(u64 depth, const std::wstring& prefix, bool is_last) {
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
			<< escape_string(m_filepath)
			<< color::white
			<< "'\n";
	}

	const std::string& file_include_node::get_filepath() const {
		return m_filepath;
	}
}
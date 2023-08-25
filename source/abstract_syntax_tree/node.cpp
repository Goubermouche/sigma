#include "node.h"
#include "abstract_syntax_tree/abstract_syntax_tree_visitor_template.h"

namespace sigma {
	node::node(const utility::file_range& range)
		: m_range(range) {}

	const utility::file_range& node::get_declared_range() const {
		return m_range;
	}

	void node::print_node_name(
		u64 depth, 
		const std::wstring& prefix,
		const std::string& node_name, 
		bool is_last
	) {
		utility::console::out
			<< prefix
			<< (depth == 0 ? L"" : is_last ? L"╰─" : L"├─")
			<< node_name
			<< ' ';
	}

	std::wstring node::get_new_prefix(
		u64 depth, 
		const std::wstring& prefix,
		bool is_last
	) {
		return depth == 0 ? L"" : prefix + (is_last ? L"  " : L"│ ");
	}
}

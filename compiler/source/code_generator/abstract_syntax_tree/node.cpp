﻿#include "node.h"
#include "code_generator/code_generator_template.h"

namespace sigma {
	node::node(const file_position& position)
		: m_position(position) {}

	const file_position& node::get_declared_position() const {
		return m_position;
	}

	void node::print_node_name(
		u64 depth, 
		const std::wstring& prefix,
		const std::string& node_name, 
		bool is_last
	) {
		console::out
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

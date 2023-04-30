#include "string_node.h"
#include "../../../../../utility/console.h"

namespace channel {
	string_node::string_node(u64 line_number, const std::string& value)
		: node(line_number), m_value(value) {}

	bool string_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(string_node);
		return visitor.visit_keyword_string_node(*this, out_value);
	}

	void string_node::print(int depth, const std::wstring& prefix, bool is_last) {
		print_value(depth, prefix, L"StringLiteral", "'char[" + std::to_string(m_value.size()) + "]' '" + escape_string(m_value) + "'\n", is_last);
	}

	const std::string& string_node::get_value() const {
		return m_value;
	}
}
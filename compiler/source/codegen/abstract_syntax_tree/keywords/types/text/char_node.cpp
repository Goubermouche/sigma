#include "char_node.h"

namespace channel {
	char_node::char_node(u64 line_number, char value)
		: node(line_number), m_value(value)	{}

	bool char_node::accept(visitor& visitor, value*& out_value)	{
		LOG_NODE_NAME(char_node);
		return visitor.visit_keyword_char_node(*this, out_value);
	}

	std::string char_node::get_node_name() const {
		return "char_node";
	}

	char char_node::get_value() const {
		return m_value;
	}
}
#include "break_node.h"

namespace channel {
	break_node::break_node(u64 line_number)
		: node(line_number) {}

	bool break_node::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(break_node);
		return visitor.visit_break_node(*this, out_value);
	}

	std::string break_node::get_node_name() const {
		return "break_node";
	}
}
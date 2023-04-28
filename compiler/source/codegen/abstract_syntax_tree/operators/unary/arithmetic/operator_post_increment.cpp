#include "operator_post_increment.h"

namespace channel {
	operator_post_increment::operator_post_increment(u64 line_number, node* expression_node)
		: operator_unary(line_number, expression_node) {}

	bool operator_post_increment::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(operator_post_increment);
		return visitor.visit_operator_post_increment_node(*this, out_value);
	}

	std::string operator_post_increment::get_node_name() const {
		return "operator_post_increment";
	}
}

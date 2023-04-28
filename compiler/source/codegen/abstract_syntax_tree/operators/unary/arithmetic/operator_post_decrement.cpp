#include "operator_post_decrement.h"

namespace channel {
	operator_post_decrement::operator_post_decrement(u64 line_number, node* expression_node)
		: operator_unary(line_number, expression_node) {}

	bool operator_post_decrement::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(operator_post_decrement);
		return visitor.visit_operator_post_decrement_node(*this, out_value);
	}

	std::string operator_post_decrement::get_node_name() const {
		return "operator_post_decrement";
	}
}

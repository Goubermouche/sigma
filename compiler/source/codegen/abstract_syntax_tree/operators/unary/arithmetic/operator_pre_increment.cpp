#include "operator_pre_increment.h"

namespace channel {
	operator_pre_increment::operator_pre_increment(u64 line_number, node* expression_node)
		: operator_unary(line_number, expression_node) {}

	bool operator_pre_increment::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(operator_pre_increment);
		return visitor.visit_operator_pre_increment_node(*this, out_value);
	}

	std::string operator_pre_increment::get_node_name() const {
		return "operator_pre_increment";
	}
}
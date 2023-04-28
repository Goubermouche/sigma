#include "operator_pre_decrement.h"

namespace channel {
	operator_pre_decrement::operator_pre_decrement(u64 line_number, node* expression_node)
		: operator_unary(line_number, expression_node) {}

	bool operator_pre_decrement::accept(visitor& visitor, value*& out_value) {
		LOG_NODE_NAME(operator_pre_decrement);
		return visitor.visit_operator_pre_decrement_node(*this, out_value);
	}

	std::string operator_pre_decrement::get_node_name() const {
		return "operator_pre_decrement";
	}
}
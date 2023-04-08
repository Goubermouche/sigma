#include "operator_modulo_node.h"

namespace channel {
	operator_modulo_node::operator_modulo_node(u64 line_index, node* left, node* right)
		: operator_binary(line_index, left, right) {}

	value* operator_modulo_node::accept(visitor& visitor) {
		LOG_NODE_NAME(operator_modulo_node);
		return visitor.visit_operator_modulo_node(*this);
	}

	std::string operator_modulo_node::get_node_name() const {
		return "operator_modulo";
	}
}
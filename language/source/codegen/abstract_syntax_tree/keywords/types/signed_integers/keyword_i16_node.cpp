#include "keyword_i16_node.h"

namespace channel {
	keyword_i16_node::keyword_i16_node(i16 value)
		: m_value(value) {}

	llvm::Value* keyword_i16_node::accept(visitor& visitor) {
		LOG_NODE_NAME(keyword_i16_node);
		return visitor.visit_keyword_i16_node(*this);
	}

	std::string keyword_i16_node::get_node_name() const {
		return "keyword_i16_node";
	}

	bool keyword_i16_node::is_signed() const {
		return true;
	}

	i16 keyword_i16_node::get_value() const {
		return m_value;
	}
}
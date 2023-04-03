#include "keyword_i8_node.h"

namespace channel {
	llvm::Value* keyword_i8_node::accept(visitor& visitor) {
		LOG_NODE_NAME(keyword_i8_node);
		return visitor.visit_keyword_i8_node(*this);
	}
}
#include "keyword_i64_node.h"

namespace channel {
	llvm::Value* keyword_i64_node::accept(visitor& visitor) {
		LOG_NODE_NAME(keyword_i64_node);
		return visitor.visit_keyword_i64_node(*this);
	}
}
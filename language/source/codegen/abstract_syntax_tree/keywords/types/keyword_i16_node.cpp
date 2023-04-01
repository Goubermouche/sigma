#include "keyword_i16_node.h"

namespace channel {
	llvm::Value* keyword_i16_node::accept(visitor& visitor) {
		return visitor.visit_keyword_i16_node(*this);
	}
}
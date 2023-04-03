#include "keyword_i16_node.h"

namespace channel {
	llvm::Value* keyword_i16_node::accept(visitor& visitor) {
		std::cout << "accepting keyword_i16_node\n";
		return visitor.visit_keyword_i16_node(*this);
	}
}
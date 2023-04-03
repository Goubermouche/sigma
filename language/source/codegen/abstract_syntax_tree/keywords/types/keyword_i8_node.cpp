#include "keyword_i8_node.h"

namespace channel {
	llvm::Value* keyword_i8_node::accept(visitor& visitor) {
		std::cout << "accepting keyword_i8_node\n";
		return visitor.visit_keyword_i8_node(*this);
	}
}
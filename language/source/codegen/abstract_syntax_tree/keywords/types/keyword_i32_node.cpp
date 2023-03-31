#include "keyword_i32_node.h"
#include "../../../visitor.h"

namespace channel {
	llvm::Value* keyword_i32_node::accept(visitor& visitor)	{
		return visitor.visit_keyword_i32_node(*this);
	}
}
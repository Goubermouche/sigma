#include "keyword_i32_node.h"

namespace channel {
	void keyword_i32_node::accept(visitor& visitor)	{
		visitor.visit_keyword_i32_node(*this);
	}
}
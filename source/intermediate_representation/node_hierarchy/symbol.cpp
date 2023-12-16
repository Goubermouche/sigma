#include "symbol.h"
#include "intermediate_representation/node_hierarchy/global.h"
#include "intermediate_representation/node_hierarchy/function.h"

namespace sigma::ir {
	auto symbol::is_non_local() const -> bool {
		if (tag == GLOBAL) {
			return ((global*)this)->link == linkage::PUBLIC;
		}

		if (tag == FUNCTION) {
			return ((function*)this)->link == linkage::PUBLIC;
		}

		return true;
	}
} // namespace sigma::ir

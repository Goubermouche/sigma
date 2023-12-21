#include "symbol.h"
#include "intermediate_representation/node_hierarchy/function.h"

namespace sigma::ir {
	symbol::symbol(symbol_type type, const std::string& name, handle<module> parent_module, linkage linkage)
		: type(type), name(name), parent_module(parent_module), link(linkage) {}

	auto symbol::is_non_local() const -> bool {
		if (type == GLOBAL || type == FUNCTION) {
			return link == linkage::PUBLIC;
		}

		return true;
	}
} // namespace sigma::ir

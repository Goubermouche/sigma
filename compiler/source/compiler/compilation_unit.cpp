#include "compilation_unit.h"

namespace sigma {
	outcome::result<void> compilation_unit::compile(
		const std::vector<std::shared_ptr<llvm_context>>& dependencies
	) {
		return outcome::success();
	}

	std::shared_ptr<llvm_context> compilation_unit::get_context() const {
		return m_context;
	}
}

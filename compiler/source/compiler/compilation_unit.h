#pragma once
#include "llvm_wrappers/llvm_context.h"
#include "utility/diagnostics/error.h"

namespace sigma {
	class compilation_unit {
	public:
		outcome::result<void> compile(
			const std::vector<std::shared_ptr<llvm_context>>& dependencies
		);

		std::shared_ptr<llvm_context> get_context() const;
	private:
		std::shared_ptr<llvm_context> m_context;
	};
}
